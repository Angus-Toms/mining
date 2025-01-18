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

        computeEntropies();
    }

    void loadExtension() {
        std::string loadQry = "LOAD './mining_extension/build/release/extension/quack/quack.duckdb_extension';";

        auto loadResult = conn.Query(loadQry);

        if (loadResult->HasError()) {
            std::string loadErrMsg = "\033[1;31mFailed to load mining extension: \033[0m";
            std::cerr << loadErrMsg << loadResult->ToString();
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
    }

    std::vector<int> runLevelQuery(std::vector<int> atts, int level) {
        if (level > atts.size()) {
            std::cerr << "\033[1;31mLevel exceeds attribute set size, cannot form combinations\033[0m\n";
        }

        std::string qry = "SELECT prune(custom_sum(lift_exact([";
        for (const auto& att : atts) {
            qry += "col" + std::to_string(att);
            if (att != *atts.rbegin()) {
                qry += ",";
            }
        }
        qry += "], " + std::to_string(level) + "))) FROM tbl;";
        conn.Query(qry)->Print();
        return {0};
    }

    void computeEntropies() {
        conn.Query("SELECT DISTINCT ON (function_name) function_name, return_type FROM duckdb_functions() ORDER BY function_name;")->Print();

        std::vector<int> atts;
        for (int i = 0; i < attributeCount; i++) {
            atts.push_back(i);
        }
        runLevelQuery(atts, 1);
        runLevelQuery(atts, 2);
        runLevelQuery(atts, 3);
        runLevelQuery(atts, 4);
        runLevelQuery(atts, 5);
    }
};

int main() {
    SchemaMiner sm("test.csv", 5);

    return 0;
}