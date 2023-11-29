import os
from pathlib import Path

import pytest

import duckdb

DEFAULT_EXTENSION_PATH = (
    Path(__file__).parent.parent.parent
    / "build/debug/extension/postal/postal.duckdb_extension"
)


# Get a fresh connection to DuckDB with the extension binary loaded
@pytest.fixture
def duckdb_conn():
    extension_binary = os.environ.get(
        "POSTAL_EXTENSION_BINARY_PATH", DEFAULT_EXTENSION_PATH
    )
    conn = duckdb.connect("", config={"allow_unsigned_extensions": "true"})
    conn.execute(f"load '{extension_binary}'")
    return conn


def test_postal(duckdb_conn):
    duckdb_conn.execute("SELECT postal('Sam') as value;")
    res = duckdb_conn.fetchall()
    assert res[0][0] == "Postal Sam 🐥"
