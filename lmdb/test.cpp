#include <lmdb.h>
#include <cstdio>
#include <iostream>
#include <cstring>
using namespace std;

#define LOG(...) { std::cout << "[log] " << __VA_ARGS__ << "\n"; }

bool insert_data(MDB_txn* txn, MDB_dbi dbi, const char* key, const char* value) {
    MDB_val mdb_key, mdb_value;

    mdb_key.mv_size = strlen(key);
    mdb_key.mv_data = (void*) key;

    mdb_value.mv_size = strlen(value);
    mdb_value.mv_data = (void*) value;

    int ret = mdb_put(txn, dbi, &mdb_key, &mdb_value, MDB_NODUPDATA);
    return ret == MDB_SUCCESS || ret == MDB_KEYEXIST;
}

string get_string(MDB_val item) {
    return string((char*) item.mv_data, item.mv_size);
}

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
    if(MDB_SUCCESS != mdb_dbi_open(txn, "offsets", MDB_CREATE | MDB_DUPSORT, &dbi)) {
        LOG("mdb_dbi_open failed");
        return 1;
    }

    if(!insert_data(txn, dbi, "00000", "key zero")) {
        LOG("insert_data #1 failed");
        return 1;
    }

    if(!insert_data(txn, dbi, "00005", "key one")) {
        LOG("insert_data #2 failed");
        return 1;
    }

    if(!insert_data(txn, dbi, "00010", "key two")) {
        LOG("insert_data #3 failed");
        return 1;
    }

    mdb_txn_commit(txn);

    // Now, dump all items

    MDB_cursor* cursor = nullptr;
    MDB_val key, data;
    int rc;

    mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn);
    mdb_cursor_open(txn, dbi, &cursor);

    char buf[128];
    char buf2[128];

	while((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT)) == 0) {
        strncpy(buf, (char*) key.mv_data, key.mv_size);
        strncpy(buf2, (char*) data.mv_data, data.mv_size);

        buf[key.mv_size] = 0;
        buf2[data.mv_size] = 0;

        LOG("dump: key=" << buf << ", value=" << buf2);
	}

    mdb_cursor_close(cursor);
    mdb_txn_abort(txn);

    // search through all items

    mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn);
    mdb_cursor_open(txn, dbi, &cursor);

    key.mv_size = 5;
    key.mv_data = (void*) "00010";

    rc = mdb_cursor_get(cursor, &key, &data, MDB_SET_RANGE);
    if(rc != MDB_SUCCESS) {
        LOG("cursor get failed, NOTFOUND=" << (rc == MDB_NOTFOUND));
        return 1;
    }

    LOG("search done.");

    std::string skey = get_string(key), svalue = get_string(data);
    LOG("found key=" << skey << ", value=" << svalue);

    mdb_cursor_close(cursor);
    mdb_txn_abort(txn);

    LOG("done.");
    return 0;
}
