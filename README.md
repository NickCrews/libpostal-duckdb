# Libpostal-Duckdb

This repository is based on https://github.com/duckdb/extension-template, check it out if you want to build and ship your own DuckDB extension.

---

Note: A peer project is https://github.com/Maxxen/duckdb-postal, which currently does this a lot better than this repo.
I probably develop off of that version if I start working
on this again. But [there are some problems with global state in libpostal](https://github.com/Maxxen/duckdb-postal/issues/1) that
don't play well with DuckDB that probably need to get resolved first, and that looks like a lot of work, so I don't have any short-term
plans to work more on this.

This project is a DuckDB extension that wraps the
[libpostal](https://github.com/openvenues/libpostal) C library,
allowing you to parse and normalize addresses from within DuckDB.

This extension provides a single scalar function:
`parse_address(string) -> array<struct<token:string, label:str>>`
```
D select parse_address('123 Wallaby Way, Sydney, Austra') as result;
┌───────────────┐
│    result     │
│    varchar    │
├───────────────┤
│ Quack Jane 🐥 │
└───────────────┘
```

## Building
### Managing dependencies

`libpostal` is not included in vcpkg, so I included it via git submodules.
First step is to build and install `libpostal` with `make libpostal-build`.
Then, continue with the rest of the following steps.

### Build steps
Now to build the extension, run:
```sh
make
```
The main binaries that will be built are:
```sh
./build/release/duckdb
./build/release/test/unittest
./build/release/extension/postal/postal.duckdb_extension
```
- `duckdb` is the binary for the duckdb shell with the extension code automatically loaded.
- `unittest` is the test runner of duckdb. Again, the extension is already linked into the binary.
- `postal.duckdb_extension` is the loadable binary as it would be distributed.

## Running the tests
Different tests can be created for DuckDB extensions.
The primary way of testing DuckDB extensions should be the SQL tests in `./test/sql`.
These SQL tests can be run using:
```sh
make test
```

### Installing the deployed binaries
To install your extension binaries from S3, you will need to do two things.
Firstly, DuckDB should be launched with the `allow_unsigned_extensions` option set to true.
How to set this will depend on the client you're using. Some examples:

CLI:
```shell
duckdb -unsigned
```

Python:
```python
con = duckdb.connect(':memory:', config={'allow_unsigned_extensions' : 'true'})
```

NodeJS:
```js
db = new duckdb.Database(':memory:', {"allow_unsigned_extensions": "true"});
```

Secondly, you will need to set the repository endpoint in DuckDB to the HTTP url of your bucket + version of the extension
you want to install. To do this run the following SQL query in DuckDB:
```sql
SET custom_extension_repository='bucket.s3.eu-west-1.amazonaws.com/<your_extension_name>/latest';
```
Note that the `/latest` path will allow you to install the latest extension version available for your current version of
DuckDB. To specify a specific version, you can pass the version instead.

After running these steps, you can install and load your extension using the regular INSTALL/LOAD commands in DuckDB:
```sql
INSTALL postal
LOAD postal
```
