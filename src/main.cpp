#include "duckdb.hpp"

int main() {
    duckdb::DBConfig config;
    config.options.allow_unsigned_extensions = true;

    duckdb::DuckDB db(":memory:", &config);
    duckdb::Connection con(db);

    con.Query("CREATE TABLE tbl(i VARCHAR);");
    con.Query("INSERT INTO tbl VALUES ('42'), ('43'), ('44');");

    //Load library
    con.Query("INSTALL './mining_extension/build/release/extension/quack/quack.duckdb_extension';")->Print();
    con.Query("LOAD quack;")->Print();

    // Test quack
    con.Query("SELECT lift([i]) FROM tbl;")->Print();
    con.Query("SELECT get_entropy(custom_sum(lift([i]))) FROM tbl;")->Print();

    return 0;
}