#include "duckdb.hpp"

#include <iostream>
#include <string>
#include <set>
#include <map>

using AttributeSet = std::set<int>;

class SchemaMiner {
private:
    static duckdb::DBConfig* initConfig() {
        auto config = new duckdb::DBConfig();
        config->options.allow_unsigned_extensions = true;
        return config;
    }

protected:
    // DB
    duckdb::DuckDB db;
    duckdb::Connection conn;

    // Relation info 
    std::string csvPath;
    int attributeCount;
    long tupleCount;

    // Entropies 
    std::map<AttributeSet, double> entropies;

public:
    SchemaMiner(std::string csvPath, int attributeCount) : 
        csvPath(csvPath),
        attributeCount(attributeCount),
        db(":memory:", initConfig()),
        conn(db) {

        // Load extension and CSV
        loadExtension();
        loadCSV();
    }

    void loadExtension() {
        std::string installQry = "INSTALL './mining_extension/build/release/extension/quack/quack.duckdb_extension';";
        installQry = "Not the correct path lol;";
        std::string loadQry = "LOAD quack;";

        auto installResult = conn.Query(installQry);
        auto loadResult = conn.Query(loadQry);

        if (installResult->HasError() || loadResult->HasError()) {
            std::string loadErrMsg = "\033[1;31mFailed to load mining extension: \033[0m";
            std::cerr << loadErrMsg << installResult->ToString() << "\n";
            exit(1);
        }
    }

    void loadCSV() {
        std::string loadQry = "CREATE TABLE tbl AS SELECT * FROM read_csv('" + csvPath + "', header=false, columns={";
        for (int i = 0; i < attributeCount; i++) {
            loadQry += "'col" + std::to_string(i) + "': 'VARCHAR'";
            if (i != attributeCount - 1) {
                loadQry += ",";
            }
        }
        loadQry += "});";
        conn.Query(loadQry);

        conn.Query("SELECT * FROM tbl;")->Print();
    }
};

int main() {
    SchemaMiner sm("test.csv", 3);

    return 0;
}