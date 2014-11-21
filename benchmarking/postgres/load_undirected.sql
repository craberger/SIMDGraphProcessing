DROP TABLE IF EXISTS edge;
CREATE TABLE edgeRaw(a BIGINT, b BIGINT);
CREATE TABLE edge(a BIGINT, b BIGINT);

COPY edgeRaw FROM :'dataset';

INSERT INTO edge(a, b) (
   SELECT DISTINCT LEAST(a, b), GREATEST(a, b) FROM edgeRaw
);

CREATE INDEX edge_a_index ON edge(a);
CREATE INDEX edge_b_index ON edge(b);

DROP TABLE IF EXISTS edgeRaw;