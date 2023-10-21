CREATE EXTENSION IF NOT EXISTS db721_fdw;
CREATE SERVER IF NOT EXISTS db721_server FOREIGN DATA WRAPPER db721_fdw;
CREATE FOREIGN TABLE IF NOT EXISTS db721_farm
(
    farm_name       varchar,
    min_age_weeks   real,
    max_age_weeks   real
) SERVER db721_server OPTIONS
(
    filename '/tmp/testdata/data-farms.db721',
    tablename 'Farm'
);

SELECT * FROM db721_farm;