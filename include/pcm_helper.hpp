#ifndef _PCM_HELPER_HPP_
#define _PCM_HELPER_HPP_

#include "common.hpp"

#ifdef ENABLE_PCM

#define READ 0
#define WRITE 1
#define PARTIAL 2

#include <cpucounters.h>
#include <iostream>

namespace pcm_helper {
   const uint32 max_imc_channels = 8;
   typedef SystemCounterState system_counter_state_t;

   typedef struct {
      ServerUncorePowerState* state;
      uint64_t time;
   } server_uncore_power_state_t;

   PCM* pcm_m = NULL;

   int32_t pcm_init() {
      pcm_m = PCM::getInstance();
      pcm_m->disableJKTWorkaround();
      int32_t ret_val = 0;

      switch(pcm_m->program()) {
         case PCM::Success:
            cout << "PCM initialized" << endl;
            break;
         case PCM::PMUBusy:
            pcm_m->resetPMU();
            ret_val = -1;
            break;
         default:
            ret_val = -1;
            break;
      }

      return ret_val;
   }

   int32_t pcm_cleanup() {
      pcm_m->resetPMU();
      return 0;
   }

   system_counter_state_t pcm_get_counter_state() {
      return getSystemCounterState();
   }

   server_uncore_power_state_t* pcm_get_uncore_power_state() {
      server_uncore_power_state_t* result = new server_uncore_power_state_t;
      ServerUncorePowerState* state = new ServerUncorePowerState[pcm_m->getNumSockets()];
      for(uint32_t i = 0; i < pcm_m->getNumSockets(); i++) {
         state[i] = pcm_m->getServerUncorePowerState(i);
      }
      result->state = state;
      result->time = pcm_m->getTickCount();
      return result;
   }

   void pcm_print_uncore_power_state(server_uncore_power_state_t* before_uncstate, server_uncore_power_state_t* after_uncstate) {
      std::cout << std::endl << "Memory bandwidth" << std::endl;

      ServerUncorePowerState* uncState1 = before_uncstate->state;
      ServerUncorePowerState* uncState2 = after_uncstate->state;
      uint64_t elapsedTime = after_uncstate->time - before_uncstate->time;

      // The following code is heavly inspired (read: copied from)
      // pcm-memory.cpp in the Intel PCM source
      for(uint32_t skt = 0; skt < pcm_m->getNumSockets(); ++skt) {
         float iMC_Rd_socket = 0.0;
         float iMC_Wr_socket = 0.0;
         uint64_t partial_write = 0;

         for(uint32_t channel = 0; channel < max_imc_channels; ++channel) {
            float iMC_Rd_socket_chan = 0.0;
            float iMC_Wr_socket_chan = 0.0;

            //In case of JKT-EN, there are only three channels. Skip one and continue.
            if(getMCCounter(channel,READ,uncState1[skt],uncState2[skt]) == 0.0 && getMCCounter(channel,WRITE,uncState1[skt],uncState2[skt]) == 0.0) {
               continue;
            }

            iMC_Rd_socket_chan = (float) (getMCCounter(channel,READ,uncState1[skt],uncState2[skt]) * 64 / 1000000.0 / (elapsedTime/1000.0));
            iMC_Wr_socket_chan = (float) (getMCCounter(channel,WRITE,uncState1[skt],uncState2[skt]) * 64 / 1000000.0 / (elapsedTime/1000.0));

            iMC_Rd_socket += iMC_Rd_socket_chan;
            iMC_Wr_socket += iMC_Wr_socket_chan;

            partial_write += (uint64_t) (getMCCounter(channel,PARTIAL,uncState1[skt],uncState2[skt]) / (elapsedTime/1000.0));

            std::cout
               << "Socket " << skt << ", channel " << channel
               << " (read/write): " << iMC_Rd_socket_chan
               << "/" << iMC_Wr_socket_chan << std::endl;
         }
      }
   }

   void pcm_print_counter_stats(system_counter_state_t before_sstate, system_counter_state_t after_sstate) {
      std::cout
         << "Instructions per clock: " << getIPC(before_sstate, after_sstate) << std::endl
         << "L2 cache hit ratio: " << getL2CacheHitRatio(before_sstate, after_sstate) << std::endl
         << "L3 cache hit ratio: " << getL3CacheHitRatio(before_sstate, after_sstate) << std::endl
         << "L2 cache misses: " << getL2CacheMisses(before_sstate, after_sstate) << std::endl
         << "L3 cache misses: " << getL3CacheMisses(before_sstate, after_sstate) << std::endl
         << "Cycles lost due to L2 cache misses: " << getCyclesLostDueL2CacheMisses(before_sstate, after_sstate) << std::endl
         << "Cycles lost due to L3 cache misses: " << getCyclesLostDueL3CacheMisses(before_sstate, after_sstate) << std::endl
         << "Bytes read: " << getBytesReadFromMC(before_sstate, after_sstate) << std::endl;
   }
}
#else
namespace pcm_helper {
   // Stubs if PCM is not included
   typedef int32_t system_counter_state_t;
   typedef int32_t server_uncore_power_state_t;
   int32_t pcm_init() { return 0; }
   int32_t pcm_cleanup() { return 0; }
   system_counter_state_t pcm_get_counter_state() { return 0; }
   server_uncore_power_state_t* pcm_get_uncore_power_state() { return 0; }
   void pcm_print_uncore_power_state(server_uncore_power_state_t* before_uncstate, server_uncore_power_state_t* after_uncstate) {}
   void pcm_print_counter_stats(system_counter_state_t before_sstate, system_counter_state_t after_sstate) {}
}
#endif

#endif
