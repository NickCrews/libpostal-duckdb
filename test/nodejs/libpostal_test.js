var duckdb = require('../../duckdb/tools/nodejs');
var assert = require('assert');

describe(`postal extension`, () => {
    let db;
    let conn;
    before((done) => {
        db = new duckdb.Database(':memory:', {"allow_unsigned_extensions":"true"});
        conn = new duckdb.Connection(db);
        conn.exec(`LOAD '${process.env.POSTAL_EXTENSION_BINARY_PATH}';`, function (err) {
            if (err) throw err;
            done();
        });
    });

    it('parse_address() function should return expected string', function (done) {
        db.all("SELECT parse_address('123 wallaby way, sydney') as value;", function (err, res) {
            if (err) throw err;
            assert.deepEqual(res, [{value: "house_number"}]);
            done();
        });
    });
});