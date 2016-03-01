#include <lmdb.h>
#include <cstdio>
#include <iostream>

#define LOG(...) { std::cout << "[log] " << __VA_ARGS__ << "\n"; }

int main() {
    LOG("Using " << MDB_VERSION_STRING);

    MDB_env* env = nullptr;
    mdb_env_create(&env);
    mdb_env_set_maxreaders(env, 1);
    mdb_env_set_mapsize(env, 1 * 1024 * 1024);
    mdb_env_set_maxdbs(env, 4);
    if(MDB_SUCCESS != mdb_env_open(env, "testdb", MDB_FIXEDMAP | MDB_NOSYNC, 0664)) {
        LOG("mdb_env_open failed");
        return 1;
    }

    MDB_txn* txn = nullptr;
    if(MDB_SUCCESS != mdb_txn_begin(env, nullptr, 0, &txn)) {
        LOG("mdb_txn_begin failed");
        return 1;
    }

    MDB_dbi dbi;
    if(MDB_SUCCESS != mdb_dbi_open(txn, "id2", MDB_CREATE | MDB_DUPSORT, &dbi)) {
        LOG("mdb_dbi_open failed");
        return 1;
    }

    LOG("env=" << env);
    return 0;
}
