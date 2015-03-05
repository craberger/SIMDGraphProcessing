#define WRITE_VECTOR 1

#include "SparseMatrix.hpp"
#include "MutableGraph.hpp"
#include "pcm_helper.hpp"
#include "Parser.hpp"

using namespace pcm_helper;

template<class T, class R>
class application{
  public:
    SparseMatrix<T,R>* graph;
    MutableGraph *inputGraph;
    size_t num_threads;
    string layout;
    size_t depth;
    long start_node;

    application(Parser input_data) {
      inputGraph = input_data.input_graph; 
      num_threads = input_data.num_threads;
      layout = input_data.layout;
      depth = input_data.n;
      start_node = input_data.start_node;
    }

#ifdef ATTRIBUTES
    inline bool myNodeSelection(uint32_t node, uint32_t attribute){
      (void)node; (void) attribute;
      return attribute > 500;
    }
    inline bool myEdgeSelection(uint32_t src, uint32_t dst, uint32_t attribute){
      (void) attribute; (void) src; (void) dst;
      return attribute == 2012;
    }
#else
    inline bool myNodeSelection(uint32_t node, uint32_t attribute){
      (void)node; (void) attribute;
      return true;
    }
    inline bool myEdgeSelection(uint32_t node, uint32_t nbr, uint32_t attribute){
      (void) node; (void) nbr; (void) attribute;
      return true;
    }
#endif

    inline void produceSubgraph(){
      auto node_selection = std::bind(&application::myNodeSelection, this, _1, _2);
      auto edge_selection = std::bind(&application::myEdgeSelection, this, _1, _2, _3);

      graph = SparseMatrix<T,R>::from_asymmetric_graph(inputGraph,node_selection,edge_selection,num_threads);
    }

  inline void queryOver(uint32_t start_node){
    uint8_t *f_data = new uint8_t[graph->matrix_size*sizeof(uint64_t)];
    uint32_t *start_array = new uint32_t[1];
    start_array[0] = start_node;
    cout << "Start node: " << start_node << endl;

    const size_t bs_size = (((graph->matrix_size + 64) / 64) * 8) + 8;

    //allocate a new visited array and set the start node
    Set<bitset> visited(bs_size);
    Set<bitset> old_visited(bs_size);
    uint64_t* old_visited_data = (uint64_t*)(old_visited.data + sizeof(uint64_t));

    bitset::set(start_node,(uint64_t*)(visited.data+sizeof(uint64_t)),0);
    Set<uinteger> frontier = Set<uinteger>::from_array(f_data,start_array,1);
    bool dense_frontier = false;

    Set<bitset> **vis_bufs = new Set<bitset>*[num_threads * PADDING];
    for(size_t i = 0; i < num_threads; i ++){
      vis_bufs[i * PADDING] = new Set<bitset>(bs_size);
    }

    double sum_union = 0.0;
    double sum_difference = 0.0;
    double sum_merge = 0.0;
    double sum_copy_time = 0.0;

    size_t path_length = 0;
    double pure_bfs_time = common::startClock();
    while(true){
      //cout << endl << " Path: " << path_length << " F-TYPE: " << frontier.type <<  " CARDINALITY: " << frontier.cardinality << " DENSITY: " << dense_frontier << endl;

      double copy_time = common::startClock();
      old_visited.copy_from(visited);
      //sum_copy_time += common::stopClock("copy time",copy_time);

      double union_time = common::startClock();
      size_t real_num_threads = num_threads;
      /*
      if(dense_frontier){
        real_num_threads = common::par_for_range(num_threads, 0, graph->matrix_size, 2048,
          [&](size_t tid, size_t i) {
            Set<bitset>* const visitedT = vis_bufs[tid * PADDING];
            uint64_t* const visitedT_data = (uint64_t*)(visitedT->data + sizeof(uint64_t));
            if(!bitset::is_set(i, old_visited_data, 0)) {
               Set<T> innbrs = this->graph->get_column(i);
               innbrs.foreach_until([&] (uint32_t nbr) -> bool {
                if(bitset::is_set(nbr, old_visited_data, 0)){
                  bitset::set(i, visitedT_data, 0);
                  //visitedT->cardinality++;
                  return true;
                }
                return false;
               });
             }
          }
        );
      } else{
        */
        //double start_uint_union = common::startClock();
        real_num_threads = frontier.par_foreach(num_threads,
          [&] (size_t tid,uint32_t n){
            Set<T> outnbrs = this->graph->get_row(n);
            ops::set_union(vis_bufs[tid * PADDING], &outnbrs);
        });
        //common::stopClock("uint union", start_uint_union);
      //}

      //double merge_time = common::startClock();
      size_t left_to_merge = real_num_threads;
      while(left_to_merge > 1) {
        for(size_t i = 0; i < left_to_merge / 2; i++){
          ops::set_union(vis_bufs[i * PADDING], vis_bufs[(left_to_merge - i - 1) * PADDING]);
        }
        left_to_merge = left_to_merge / 2 + (left_to_merge % 2);
      }
      ops::set_union(&visited, vis_bufs[0]);
      //sum_merge += common::stopClock("merge_time", merge_time);

      //sum_union += common::stopClock("union time",union_time);

      //double diff_time = common::startClock();
      frontier = *ops::set_difference(&frontier,&visited,&old_visited);
      //sum_difference += common::stopClock("difference",diff_time);

      if(frontier.cardinality == 0 || path_length >= depth)
        break;
      path_length++;

      //TO TURN BEAMER OFF COMMENT OUT THIS LINE
      dense_frontier = false;//(double)frontier.cardinality / graph->matrix_size) > 0.06;
    }
    common::stopClock("pure bfs time", pure_bfs_time);

    cout << "path length: " << path_length << endl;
    cout << "frontier size: " << frontier.cardinality << endl;

    cout << "Sum of merge: " << sum_merge << endl;
    cout << "Sum of unions: " << sum_union << endl;
    cout << "Sum of differences: " << sum_difference << endl;
    cout << "Sum of copy time: " << sum_copy_time << endl;

  }

  inline void run(){
    double selection_time = common::startClock();
    produceSubgraph();
    common::stopClock("Selections",selection_time);

    uint32_t internal_start;
    if(start_node == -1)
      internal_start = graph->get_max_row_id();
    else
      internal_start = graph->get_internal_id(start_node);
    cout << "Start node - External: " << graph->id_map[internal_start] << " Internal: " << internal_start << endl;

    if(pcm_init() < 0)
      return;

    double bfs_time = common::startClock();
    queryOver(internal_start);
    common::stopClock("BFS",bfs_time);

    pcm_cleanup();
  }
};

//Ideally the user shouldn't have to concern themselves with what happens down here.
int main (int argc, char* argv[]) { 
  Parser input_data = input_parser::parse(argc,argv,"n_path");

  if(input_data.layout.compare("uint") == 0){
    application<uinteger,uinteger> myapp(input_data);
    myapp.run();
  } else if(input_data.layout.compare("bs") == 0){
    application<bitset,bitset> myapp(input_data);
    myapp.run();  
  } else if(input_data.layout.compare("pshort") == 0){
    application<pshort,pshort> myapp(input_data);
    myapp.run();  
  } else if(input_data.layout.compare("hybrid") == 0){
    application<hybrid,hybrid> myapp(input_data);
    myapp.run();  
  } else if(input_data.layout.compare("v") == 0){
    application<variant,uinteger> myapp(input_data);
    myapp.run();  
  } else if(input_data.layout.compare("bp") == 0){
    application<bitpacked,uinteger> myapp(input_data);
    myapp.run();
  } else{
    cout << "No valid layout entered." << endl;
    exit(0);
  }
  return 0;
}
