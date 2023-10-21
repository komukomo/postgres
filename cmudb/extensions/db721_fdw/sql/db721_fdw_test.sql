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

CREATE FOREIGN TABLE IF NOT EXISTS db721_chickens
(
    age_weeks       real,
    farm_name       varchar,
    identifier      int,
    notes           varchar,
    sex             varchar,
    weight_g        real,
    weight_model    varchar
) SERVER db721_server OPTIONS
(
    filename '/tmp/testdata/data-chickens.db721',
    tablename 'Chicken'
);
SELECT * FROM db721_chickens LIMIT 10;
SELECT count(*) FROM db721_chickens;