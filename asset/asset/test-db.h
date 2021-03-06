#pragma once
#include <mariadb/mysql.h>
#include <filesystem>
#include "asset/db.h"
#include <thread>
#include <fty/expected.h>

namespace fty {

// =====================================================================================================================

class TestDb
{
public:
    Expected<std::string> create();
    Expected<void>        destroy();

private:
    std::string m_path;
};

// =====================================================================================================================

class CharArray
{
public:
    template <typename... Args>
    CharArray(const Args&... args)
    {
        add(args...);
        m_data.push_back(nullptr);
    }

    CharArray(const CharArray&) = delete;

    ~CharArray()
    {
        for (size_t i = 0; i < m_data.size(); i++) {
            delete[] m_data[i];
        }
    }

    template <typename... Args>
    void add(const std::string& arg, const Args&... args)
    {
        add(arg);
        add(args...);
    }

    void add(const std::string& str)
    {
        char* s = new char[str.size() + 1];
        memset(s, 0, str.size() + 1);
        strncpy(s, str.c_str(), str.size());
        m_data.push_back(s);
    }

    char** data()
    {
        return m_data.data();
    }

    size_t size() const
    {
        return m_data.size();
    }

private:
    std::vector<char*> m_data;
};


static void createDB()
{
    tnt::Connection conn;
    conn.execute("CREATE DATABASE IF NOT EXISTS box_utf8 character set utf8 collate utf8_general_ci;");
    conn.execute("USE box_utf8");
    conn.execute("CREATE TABLE IF NOT EXISTS t_empty (id TINYINT);");
    conn.execute(R"(
        CREATE TABLE t_bios_asset_element_type (
            id_asset_element_type TINYINT UNSIGNED NOT NULL AUTO_INCREMENT,
            name                  VARCHAR(50)      NOT NULL,
            PRIMARY KEY (id_asset_element_type),
            UNIQUE INDEX `UI_t_bios_asset_element_type` (`name` ASC)
        ) AUTO_INCREMENT = 1;
    )");

    conn.execute(R"(
        INSERT INTO t_bios_asset_element_type (name)
        VALUES  ("group"),
                ("datacenter"),
                ("room"),
                ("row"),
                ("rack"),
                ("device");
    )");

    conn.execute(R"(
        CREATE TABLE t_bios_asset_device_type(
            id_asset_device_type TINYINT UNSIGNED   NOT NULL AUTO_INCREMENT,
            name                 VARCHAR(50)        NOT NULL,
            PRIMARY KEY (id_asset_device_type),
            UNIQUE INDEX `UI_t_bios_asset_device_type` (`name` ASC)
        );
    )");

    conn.execute(R"(
        CREATE VIEW v_bios_asset_device_type AS
            SELECT id_asset_device_type as id, name FROM t_bios_asset_device_type;
    )");

    conn.execute(R"(
        CREATE TABLE t_bios_asset_element (
            id_asset_element  INT UNSIGNED        NOT NULL AUTO_INCREMENT,
            name              VARCHAR(50)         NOT NULL,
            id_type           TINYINT UNSIGNED    NOT NULL,
            id_subtype        TINYINT UNSIGNED    NOT NULL DEFAULT 11,
            id_parent         INT UNSIGNED,
            status            VARCHAR(9)          NOT NULL DEFAULT "nonactive",
            priority          TINYINT             NOT NULL DEFAULT 5,
            asset_tag         VARCHAR(50),

            PRIMARY KEY (id_asset_element),

            INDEX FK_ASSETELEMENT_ELEMENTTYPE_idx (id_type   ASC),
            INDEX FK_ASSETELEMENT_ELEMENTSUBTYPE_idx (id_subtype   ASC),
            INDEX FK_ASSETELEMENT_PARENTID_idx    (id_parent ASC),
            UNIQUE INDEX `UI_t_bios_asset_element_NAME` (`name` ASC),
            INDEX `UI_t_bios_asset_element_ASSET_TAG` (`asset_tag`  ASC),

            CONSTRAINT FK_ASSETELEMENT_ELEMENTTYPE
            FOREIGN KEY (id_type)
            REFERENCES t_bios_asset_element_type (id_asset_element_type)
            ON DELETE RESTRICT,

            CONSTRAINT FK_ASSETELEMENT_ELEMENTSUBTYPE
            FOREIGN KEY (id_subtype)
            REFERENCES t_bios_asset_device_type (id_asset_device_type)
            ON DELETE RESTRICT,

            CONSTRAINT FK_ASSETELEMENT_PARENTID
            FOREIGN KEY (id_parent)
            REFERENCES t_bios_asset_element (id_asset_element)
            ON DELETE RESTRICT
        );
    )");

    conn.execute(R"(
        CREATE TABLE t_bios_asset_ext_attributes(
            id_asset_ext_attribute    INT UNSIGNED NOT NULL AUTO_INCREMENT,
            keytag                    VARCHAR(40)  NOT NULL,
            value                     VARCHAR(255) NOT NULL,
            id_asset_element          INT UNSIGNED NOT NULL,
            read_only                 TINYINT      NOT NULL DEFAULT 0,

            PRIMARY KEY (id_asset_ext_attribute),

            INDEX FK_ASSETEXTATTR_ELEMENT_idx (id_asset_element ASC),
            UNIQUE INDEX `UI_t_bios_asset_ext_attributes` (`keytag`, `id_asset_element` ASC),

            CONSTRAINT FK_ASSETEXTATTR_ELEMENT
            FOREIGN KEY (id_asset_element)
            REFERENCES t_bios_asset_element (id_asset_element)
            ON DELETE RESTRICT
        );
    )");

    conn.execute(R"(
        INSERT INTO t_bios_asset_device_type (name)
        VALUES  ("ups"),
                ("genset"),
                ("epdu"),
                ("pdu"),
                ("server"),
                ("feed"),
                ("sts"),
                ("switch"),
                ("storage"),
                ("vm");
    )");

    conn.execute(R"(
        INSERT INTO t_bios_asset_device_type (id_asset_device_type, name) VALUES (11, "N_A");
    )");

    conn.execute(R"(
        CREATE VIEW v_bios_asset_element_type AS
            SELECT
                t1.id_asset_element_type AS id,
                t1.name
            FROM
                t_bios_asset_element_type t1;
    )");

    conn.execute(R"(
        CREATE VIEW v_bios_asset_element AS
            SELECT  v1.id_asset_element AS id,
                    v1.name,
                    v1.id_type,
                    v1.id_subtype,
                    v1.id_parent,
                    v2.id_type AS id_parent_type,
                    v2.name AS parent_name,
                    v1.status,
                    v1.priority,
                    v1.asset_tag
                FROM t_bios_asset_element v1
                LEFT JOIN  t_bios_asset_element v2
                    ON (v1.id_parent = v2.id_asset_element) ;
    )");

    conn.execute(R"(
        CREATE VIEW v_bios_asset_ext_attributes AS
            SELECT * FROM t_bios_asset_ext_attributes ;
    )");

    conn.execute(R"(
        CREATE VIEW v_web_element AS
            SELECT
                t1.id_asset_element AS id,
                t1.name,
                t1.id_type,
                v3.name AS type_name,
                t1.id_subtype AS subtype_id,
                v4.name AS subtype_name,
                t1.id_parent,
                t2.id_type AS id_parent_type,
                t2.name AS parent_name,
                t1.status,
                t1.priority,
                t1.asset_tag
            FROM
                t_bios_asset_element t1
                LEFT JOIN t_bios_asset_element t2
                    ON (t1.id_parent = t2.id_asset_element)
                LEFT JOIN v_bios_asset_element_type v3
                    ON (t1.id_type = v3.id)
                LEFT JOIN t_bios_asset_device_type v4
                    ON (v4.id_asset_device_type = t1.id_subtype);
    )");

    conn.execute(R"(
        CREATE TABLE t_bios_asset_group_relation (
          id_asset_group_relation INT UNSIGNED NOT NULL AUTO_INCREMENT,
          id_asset_group          INT UNSIGNED NOT NULL,
          id_asset_element        INT UNSIGNED NOT NULL,

          PRIMARY KEY (id_asset_group_relation),

          INDEX FK_ASSETGROUPRELATION_ELEMENT_idx (id_asset_element ASC),
          INDEX FK_ASSETGROUPRELATION_GROUP_idx   (id_asset_group   ASC),

          UNIQUE INDEX `UI_t_bios_asset_group_relation` (`id_asset_group`, `id_asset_element` ASC),

          CONSTRAINT FK_ASSETGROUPRELATION_ELEMENT
            FOREIGN KEY (id_asset_element)
            REFERENCES t_bios_asset_element (id_asset_element)
            ON DELETE RESTRICT,

          CONSTRAINT FK_ASSETGROUPRELATION_GROUP
            FOREIGN KEY (id_asset_group)
            REFERENCES t_bios_asset_element (id_asset_element)
            ON DELETE RESTRICT
        );
    )");

    conn.execute(R"(
        CREATE VIEW v_bios_asset_group_relation AS
            SELECT * FROM t_bios_asset_group_relation;
    )");

    conn.execute(R"(
        CREATE TABLE t_bios_asset_link_type(
          id_asset_link_type   TINYINT UNSIGNED   NOT NULL AUTO_INCREMENT,
          name                 VARCHAR(50)        NOT NULL,

          PRIMARY KEY (id_asset_link_type),
          UNIQUE INDEX `UI_t_bios_asset_link_type_name` (`name` ASC)

        );
    )");

    conn.execute(R"(
        INSERT INTO t_bios_asset_link_type (name) VALUES ("power chain");
    )");

    conn.execute(R"(
        CREATE TABLE t_bios_asset_link (
          id_link               INT UNSIGNED        NOT NULL AUTO_INCREMENT,
          id_asset_device_src   INT UNSIGNED        NOT NULL,
          src_out               CHAR(4),
          id_asset_device_dest  INT UNSIGNED        NOT NULL,
          dest_in               CHAR(4),
          id_asset_link_type    TINYINT UNSIGNED    NOT NULL,

          PRIMARY KEY (id_link),

          INDEX FK_ASSETLINK_SRC_idx  (id_asset_device_src    ASC),
          INDEX FK_ASSETLINK_DEST_idx (id_asset_device_dest   ASC),
          INDEX FK_ASSETLINK_TYPE_idx (id_asset_link_type     ASC),

          CONSTRAINT FK_ASSETLINK_SRC
            FOREIGN KEY (id_asset_device_src)
            REFERENCES t_bios_asset_element(id_asset_element)
            ON DELETE RESTRICT,

          CONSTRAINT FK_ASSETLINK_DEST
            FOREIGN KEY (id_asset_device_dest)
            REFERENCES t_bios_asset_element(id_asset_element)
            ON DELETE RESTRICT,

          CONSTRAINT FK_ASSETLINK_TYPE
            FOREIGN KEY (id_asset_link_type)
            REFERENCES t_bios_asset_link_type(id_asset_link_type)
            ON DELETE RESTRICT

        );
    )");

    conn.execute(R"(
        CREATE VIEW v_bios_asset_link AS
            SELECT  v1.id_link,
                    v1.src_out,
                    v1.dest_in,
                    v1.id_asset_link_type,
                    v1.id_asset_device_src AS id_asset_element_src,
                    v1.id_asset_device_dest AS id_asset_element_dest
            FROM t_bios_asset_link v1;
    )");

    conn.execute(R"(
        CREATE VIEW v_web_asset_link AS
            SELECT
                v1.id_link,
                v1.id_asset_link_type,
                t3.name AS link_name,
                v1.id_asset_element_src,
                t1.name AS src_name,
                v1.id_asset_element_dest,
                t2.name AS dest_name,
                v1.src_out,
                v1.dest_in
            FROM
                 v_bios_asset_link v1
            JOIN t_bios_asset_element t1
                ON v1.id_asset_element_src=t1.id_asset_element
            JOIN t_bios_asset_element t2
                ON v1.id_asset_element_dest=t2.id_asset_element
            JOIN t_bios_asset_link_type t3
                ON v1.id_asset_link_type=t3.id_asset_link_type;
    )");

    conn.execute(R"(
        CREATE VIEW v_bios_asset_device AS
            SELECT  t1.id_asset_element,
                    t2.id_asset_device_type,
                    t2.name
            FROM t_bios_asset_element t1
                LEFT JOIN t_bios_asset_device_type t2
                ON (t1.id_subtype = t2.id_asset_device_type);
    )");

    conn.execute(R"(
        CREATE VIEW v_bios_asset_element_super_parent AS
        SELECT v1.id as id_asset_element,
               v1.id_parent AS id_parent1,
               v2.id_parent AS id_parent2,
               v3.id_parent AS id_parent3,
               v4.id_parent AS id_parent4,
               v5.id_parent AS id_parent5,
               v6.id_parent AS id_parent6,
               v7.id_parent AS id_parent7,
               v8.id_parent AS id_parent8,
               v9.id_parent AS id_parent9,
               v10.id_parent AS id_parent10,
               v1.parent_name AS name_parent1,
               v2.parent_name AS name_parent2,
               v3.parent_name AS name_parent3,
               v4.parent_name AS name_parent4,
               v5.parent_name AS name_parent5,
               v6.parent_name AS name_parent6,
               v7.parent_name AS name_parent7,
               v8.parent_name AS name_parent8,
               v9.parent_name AS name_parent9,
               v10.parent_name AS name_parent10,
               v2.id_type AS id_type_parent1,
               v3.id_type AS id_type_parent2,
               v4.id_type AS id_type_parent3,
               v5.id_type AS id_type_parent4,
               v6.id_type AS id_type_parent5,
               v7.id_type AS id_type_parent6,
               v8.id_type AS id_type_parent7,
               v9.id_type AS id_type_parent8,
               v10.id_type AS id_type_parent9,
               v11.id_type AS id_type_parent10,
               v2.id_subtype AS id_subtype_parent1,
               v3.id_subtype AS id_subtype_parent2,
               v4.id_subtype AS id_subtype_parent3,
               v5.id_subtype AS id_subtype_parent4,
               v6.id_subtype AS id_subtype_parent5,
               v7.id_subtype AS id_subtype_parent6,
               v8.id_subtype AS id_subtype_parent7,
               v9.id_subtype AS id_subtype_parent8,
               v10.id_subtype AS id_subtype_parent9,
               v11.id_subtype AS id_subtype_parent10,
               v1.name ,
               v12.name AS type_name,
               v12.id_asset_device_type,
               v1.status,
               v1.asset_tag,
               v1.priority,
               v1.id_type
        FROM v_bios_asset_element v1
             LEFT JOIN v_bios_asset_element v2
                ON (v1.id_parent = v2.id)
             LEFT JOIN v_bios_asset_element v3
                ON (v2.id_parent = v3.id)
             LEFT JOIN v_bios_asset_element v4
                ON (v3.id_parent = v4.id)
             LEFT JOIN v_bios_asset_element v5
                ON (v4.id_parent = v5.id)
             LEFT JOIN v_bios_asset_element v6
                ON (v5.id_parent = v6.id)
             LEFT JOIN v_bios_asset_element v7
                ON (v6.id_parent = v7.id)
             LEFT JOIN v_bios_asset_element v8
                ON (v7.id_parent = v8.id)
             LEFT JOIN v_bios_asset_element v9
                ON (v8.id_parent = v9.id)
             LEFT JOIN v_bios_asset_element v10
                ON (v9.id_parent = v10.id)
             LEFT JOIN v_bios_asset_element v11
                ON (v10.id_parent = v11.id)
             INNER JOIN t_bios_asset_device_type v12
                ON (v12.id_asset_device_type = v1.id_subtype);
    )");

    conn.execute(R"(
        CREATE TABLE t_bios_device_type(
            id_device_type      TINYINT UNSIGNED NOT NULL AUTO_INCREMENT,
            name                VARCHAR(50)      NOT NULL,

            PRIMARY KEY(id_device_type),

            UNIQUE INDEX `UI_t_bios_device_type_name` (`name` ASC)

        ) AUTO_INCREMENT = 1;
    )");

    conn.execute(R"(
        INSERT INTO t_bios_device_type (name) VALUES
            ("not_classified"),
            ("ups"),
            ("epdu"),
            ("server");
    )");

    conn.execute(R"(
        CREATE VIEW v_bios_device_type AS
            SELECT id_device_type id, name FROM t_bios_device_type;
    )");

    conn.execute(R"(
        CREATE TABLE t_bios_discovered_device(
            id_discovered_device    SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
            name                    VARCHAR(50)       NOT NULL,
            id_device_type          TINYINT UNSIGNED  NOT NULL,

            PRIMARY KEY(id_discovered_device),

            INDEX(id_device_type),

            UNIQUE INDEX `UI_t_bios_discovered_device_name` (`name` ASC),

            FOREIGN KEY(id_device_type)
            REFERENCES t_bios_device_type(id_device_type)
                ON DELETE RESTRICT
        );
    )");

    conn.execute(R"(
        CREATE TABLE t_bios_monitor_asset_relation(
            id_ma_relation        INT UNSIGNED      NOT NULL AUTO_INCREMENT,
            id_discovered_device  SMALLINT UNSIGNED NOT NULL,
            id_asset_element      INT UNSIGNED      NOT NULL,

            PRIMARY KEY(id_ma_relation),

            INDEX(id_discovered_device),
            INDEX(id_asset_element),

            FOREIGN KEY (id_discovered_device)
                REFERENCEs t_bios_discovered_device(id_discovered_device)
                ON DELETE RESTRICT,

            FOREIGN KEY (id_asset_element)
                REFERENCEs t_bios_asset_element(id_asset_element)
                ON DELETE RESTRICT
        );
    )");
}

Expected<std::string> TestDb::create()
{
    std::stringstream ss;
    ss << getpid();
    std::string pid = ss.str();

    m_path = "/tmp/mysql-"+pid;
    std::string sock = "/tmp/mysql-"+pid+".sock";

    if (!std::filesystem::create_directory(m_path)) {
        return unexpected("cannot create db dir {}", m_path);
    }

    CharArray options("mysql_test", "--datadir="+m_path, "--socket="+sock);
    CharArray groups("libmysqld_server", "libmysqld_client");

    mysql_library_init(int(options.size())-1, options.data(), groups.data());

    std::string url = "mysql:unix_socket="+sock;
    setenv("DBURL", url.c_str(), 1);

    try {
        createDB();
    } catch (const std::exception& e) {
        return unexpected(e.what());
    }

    return "mysql:unix_socket="+sock+";db=box_utf8";
}

Expected<void> TestDb::destroy()
{
    tnt::shutdown();
    mysql_thread_end();
    mysql_library_end();

    std::filesystem::remove_all(m_path);
    return {};
}

}
