#include <leveldb/db.h>
#include <iostream>

void put_offset(leveldb::DB* db, leveldb::WriteOptions wopts, uint64_t offs, const char* data) {
    using leveldb::Slice;

    char buf[8];
    char *pb = (char*) &offs;

    buf[0] = pb[7];
    buf[1] = pb[6];
    buf[2] = pb[5];
    buf[3] = pb[4];
    buf[4] = pb[3];
    buf[5] = pb[2];
    buf[6] = pb[1];
    buf[7] = pb[0];

    Slice s(buf, 8);
    db->Put(wopts, s, data);
}

int main() {
    using leveldb::DB;
    using leveldb::Slice;
    using leveldb::Iterator;
    using std::cout;

	DB* db = nullptr;
	leveldb::Options opts;
	opts.create_if_missing = true;

	leveldb::Status status = leveldb::DB::Open(opts, "/tmp/dbtemp", &db);
	if(!db || status.ok() == false) {
		cout << "can't create database\n";
		return 1;
	}

    leveldb::WriteOptions wopts;
    leveldb::ReadOptions ropts;

    put_offset(db, wopts, 0x1000, "offs 1k");
    put_offset(db, wopts, 0x2000, "offs 2k");
    put_offset(db, wopts, 0x3000, "offs 3k");

    Iterator* iter = db->NewIterator(ropts);

    char buf[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00 };
    Slice s(buf, 8);
    iter->Seek(s);

    if(iter->Valid()) {
        cout << "Positioned itself on a valid iter\n";

        std::string value(iter->value().data(), iter->value().size());
        cout << "data: '" << value << "'\n";
    } else {
        cout << "not found using Seek()\n";
    }

	return 0;
}
