// If you choose to use C++, read this very carefully:
// https://www.postgresql.org/docs/15/xfunc-c.html#EXTEND-CPP

#include "dog.h"
#include "db721_reader.h"

// clang-format off
extern "C" {
// #include "../../../../src/include/postgres.h"
// #include "../../../../src/include/fmgr.h"
// #include "../../../../src/include/foreign/fdwapi.h"
// #include "../../../../src/include/foreign/foreign.h"
// #include "../../../../src/include/optimizer/pathnode.h"
// #include "../../../../src/include/optimizer/restrictinfo.h"
// #include "../../../../src/include/optimizer/planmain.h"
// #include "../../../../src/include/utils/builtins.h"
// #include "../../../../src/include/utils/rel.h"
// #include "../../../../src/include/access/table.h"
// #include "../../../../src/include/commands/defrem.h"

#include "postgres.h"
#include "fmgr.h"
#include "foreign/fdwapi.h"
#include "foreign/foreign.h"
#include "optimizer/pathnode.h"
#include "optimizer/restrictinfo.h"
#include "optimizer/planmain.h"
#include "utils/builtins.h"
#include "utils/rel.h"
#include "access/table.h"
#include "commands/defrem.h"
}
// clang-format on

typedef struct db721_TableOptions {
  char *filename, *tablename;
} db721_TableOptions;


extern "C" void db721_GetForeignRelSize(PlannerInfo *root, RelOptInfo *baserel,
                                      Oid foreigntableid) {
  // TODO(721): Write me!
  Dog terrier("Terrier");
  elog(LOG, "db721_GetForeignRelSize: %s", terrier.Bark().c_str());

  ForeignTable *ft = GetForeignTable(foreigntableid);
  ListCell *cell;
  char *filename = NULL;
  char *tablename = NULL;
  foreach(cell, ft->options) {
    DefElem *def = lfirst_node(DefElem, cell);
    if (strcmp("filename", def->defname) == 0) {
      filename = defGetString(def);
    }
    if (strcmp("tablename", def->defname) == 0) {
      tablename = defGetString(def);
    }
  }

  elog(LOG, "db721_GetForeignRelSize options: %s %s", filename, tablename);
  db721_TableOptions *opts = (db721_TableOptions *) palloc(sizeof(db721_TableOptions));
  opts->filename = filename;
  opts->tablename = tablename;
  baserel->fdw_private = opts;
}

extern "C" void db721_GetForeignPaths(PlannerInfo *root, RelOptInfo *baserel,
                                    Oid foreigntableid) {
  // TODO(721): Write me!
  Dog scout("Scout");
  elog(LOG, "db721_GetForeignPaths: %s", scout.Bark().c_str());

  Path *path = (Path *)create_foreignscan_path(root, baserel,
    NULL,
    baserel->rows,
    1,
    1 + baserel->rows,
    NIL,
    NULL,
    NULL,
    NIL
  );
  add_path(baserel, path);
}

extern "C" ForeignScan *
db721_GetForeignPlan(PlannerInfo *root, RelOptInfo *baserel, Oid foreigntableid,
                   ForeignPath *best_path, List *tlist, List *scan_clauses,
                   Plan *outer_plan) {
  // TODO(721): Write me!
  db721_TableOptions *opts = (db721_TableOptions *) baserel->fdw_private;
  List *fdw_private = list_make2(makeString(opts->filename), makeString(opts->tablename));
  scan_clauses = extract_actual_clauses(scan_clauses, false);
  return make_foreignscan(
    tlist,
    scan_clauses,
    baserel->relid,
    NIL,
    fdw_private,
    NIL,
    NIL,
    outer_plan
  );
}

typedef struct db721_state {
  int current;
  char *filename;
  FILE *tablefile;
  Metadata metadata;
  Db721Reader reader;
} db721_state;

extern "C" void db721_BeginForeignScan(ForeignScanState *node, int eflags) {
  db721_state *state = (db721_state*) palloc0(sizeof(db721_state));
  ForeignScan *fs = (ForeignScan *) node->ss.ps.plan;
  char *filename = strVal(linitial(fs->fdw_private));
  char *tablename = strVal(lsecond(fs->fdw_private));
  FILE *tablefile = AllocateFile(filename, PG_BINARY_R);
  auto metadata = read_metadata(tablefile);
  // elog(LOG, "reader_foo called %s", farm.columns[0].data()->name.c_str());
  // elog(LOG, "reader_foo called: %s", farm.c_str());

  if (tablefile == NULL) {
    ereport(ERROR, 
      errcode_for_file_access(),
      errmsg("could not open file \"%s\"", filename)
    );
  }
  state->tablefile = tablefile;
  state->filename = filename;
  state->metadata = metadata;
  state->reader = Db721Reader(tablefile, metadata);
  elog(LOG, "db721_BeginForeignScan options: %s %s", filename, tablename);
  node->fdw_state = state;
}

extern "C" TupleTableSlot *db721_IterateForeignScan(ForeignScanState *node) {
  // TODO(721): Write me!
  TupleTableSlot *slot = node->ss.ss_ScanTupleSlot;
  ExecClearTuple(slot);

  db721_state *state = (db721_state*) node->fdw_state;
  if (state->reader.read_next(slot->tts_values)) {
    ExecStoreVirtualTuple(slot);
    for (int i = 0; i < slot->tts_tupleDescriptor->natts; i++) {
      slot->tts_isnull[i] = false;
    }
  }
  return slot;
}

extern "C" void db721_ReScanForeignScan(ForeignScanState *node) {
  // TODO(721): Write me!
}

extern "C" void db721_EndForeignScan(ForeignScanState *node) {
  // TODO(721): Write me!
  db721_state *state = (db721_state*) node->fdw_state;
  int result = FreeFile(state->tablefile);
  if (result != 0) {
    ereport(ERROR, 
      errcode_for_file_access(),
      errmsg("could not close file \"%s\"", state->filename)
    );
  }
}