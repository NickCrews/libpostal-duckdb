var duckdb = require('../../duckdb/tools/nodejs');
var assert = require('assert');

describe(`libpostal extension`, () => {
    let db;
    let conn;
    before((done) => {
        db = new duckdb.Database(':memory:', {"allow_unsigned_extensions":"true"});
        conn = new duckdb.Connection(db);
        conn.exec(`LOAD '${process.env.QUACK_EXTENSION_BINARY_PATH}';`, function (err) {
            if (err) throw err;
            done();
        });
    });

    it('libpostal function should return expected string', function (done) {
        db.all("SELECT libpostal('Sam') as value;", function (err, res) {
            if (err) throw err;
            assert.deepEqual(res, [{value: "Libpostal Sam 🐥"}]);
            done();
        });
    });
});