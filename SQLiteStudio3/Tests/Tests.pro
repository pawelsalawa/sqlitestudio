TEMPLATE = subdirs

test_utils.subdir = TestUtils

completion_helper.subdir = CompletionHelperTest
completion_helper.depends = test_utils

select_resolver.subdir = SelectResolverTest
select_resolver.depends = test_utils

parser.subdir = ParserTest
parser.depends = test_utils

table_modifier.subdir = TableModifierTest
table_modifier.depends = test_utils

hash_tables.subdir = HashTablesTest
hash_tables.depends = test_utils

db_ver_conv.subdir = DbVersionConverterTest
db_ver_conv.depends = test_utils

dsv.subdir = DsvFormatsTest
dsv.depends = test_utils

SUBDIRS += \
    test_utils \
    completion_helper \
    select_resolver \
    parser \
    table_modifier \
    hash_tables \
    db_ver_conv \
    dsv \
    UtilsTest
