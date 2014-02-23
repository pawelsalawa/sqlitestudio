#include "strhash.h"
#include "bistrhash.h"
#include "bihash.h"
#include <QString>
#include <QtTest>
#include <QDebug>

class HashTablesTestTest : public QObject
{
        Q_OBJECT

    public:
        HashTablesTestTest();

    private Q_SLOTS:
        void strHash1();
        void strHash2();
        void strHash3();
        void strHash4();
        void strHash5();
        void strHash6();
        void biStrHash1();
        void biStrHash2();
        void biStrHash3();
        void biHash1();
        void biHash2();
};

HashTablesTestTest::HashTablesTestTest()
{
}

void HashTablesTestTest::strHash1()
{
    StrHash<QString> hash;
    hash.insert("key1", "value1");
    hash.insert("key2", "value2");
    hash.insert("KEY2", "value3");

    QVERIFY(hash.contains("KEY1", Qt::CaseInsensitive));
    QVERIFY(!hash.contains("KEY1", Qt::CaseSensitive));
    QVERIFY(hash.contains("key2", Qt::CaseInsensitive));
    QVERIFY(!hash.contains("key2", Qt::CaseSensitive));
    QVERIFY(hash.value("key2", Qt::CaseInsensitive) == "value3");
}

void HashTablesTestTest::strHash2()
{
    StrHash<QString> hash;
    hash.insert("key1", "value1");
    hash.insert("KEY2", "value2");
    hash.insert("KEY3", "value3");

    hash.remove("key2");
    QVERIFY(hash.count() == 3);

    hash.remove("key2", Qt::CaseInsensitive);
    QVERIFY(hash.count() == 2);
}

void HashTablesTestTest::strHash3()
{
    StrHash<QString> hash1;
    hash1.insert("key1", "value1");
    hash1.insert("KEY2", "value2");
    hash1.insert("KEY3", "value3");

    StrHash<QString> hash2;
    hash2.insert("key3", "value6");
    hash2.insert("KEY4", "value7");
    hash2.insert("KEY5", "value8");

    hash1.unite(hash2);

    QVERIFY(hash1.count() == 5);
    QVERIFY(hash1.contains("key3", Qt::CaseSensitive));
    QVERIFY(hash1.value("key3", Qt::CaseInsensitive) == "value6");
}

void HashTablesTestTest::strHash4()
{
    StrHash<QString> hash;
    hash["key1"] = "value1";
    hash["KEY2"] = "value2";
    hash["KEY3"] = "value3";
    hash["key3"] = "value4";

    QVERIFY(hash.count() == 3);
    QVERIFY(hash.value("key3", Qt::CaseSensitive) == "value4");
    QVERIFY(hash.values().size() == 3);
    QVERIFY(hash.keys().size() == 3);
}

void HashTablesTestTest::strHash5()
{
    StrHash<QString> hash;
    hash["key1"] = "value1";
    hash["KEY2"] = "value2";
    hash["KEY3"] = "value3";
    hash["key3"] = "value4";

    QVERIFY(hash.count("key1", Qt::CaseInsensitive) == 1);
    QVERIFY(hash.count("key2", Qt::CaseInsensitive) == 1);
    QVERIFY(hash.count("key3", Qt::CaseInsensitive) == 1);
    QVERIFY(hash.count("KEY3", Qt::CaseInsensitive) == 1);
    QVERIFY(hash.count("key1", Qt::CaseSensitive) == 1);
    QVERIFY(hash.count("key2", Qt::CaseSensitive) == 0);
    QVERIFY(hash.count("key3", Qt::CaseSensitive) == 1);
}

void HashTablesTestTest::strHash6()
{
    StrHash<QString> hash;
    hash["key1"] = "value1";
    hash["KEY2"] = "value2";
    hash["KEY3"] = "value3";

    QVERIFY(hash["key3"] == "value3");
}

void HashTablesTestTest::biStrHash1()
{
    BiStrHash hash;
    hash.insert("left1", "right1");
    hash.insert("LEFT2", "right2");
    hash.insert("left3", "RIGHT3");
    hash.insert("LEFT4", "RIGHT4");

    QVERIFY(hash.count() == 4);
    QVERIFY(hash.containsLeft("left1", Qt::CaseSensitive));
    QVERIFY(!hash.containsLeft("left2", Qt::CaseSensitive));
    QVERIFY(hash.containsLeft("left2", Qt::CaseInsensitive));
    QVERIFY(hash.containsRight("right1", Qt::CaseSensitive));
    QVERIFY(!hash.containsRight("right3", Qt::CaseSensitive));
    QVERIFY(hash.containsRight("right3", Qt::CaseInsensitive));
    QVERIFY(hash.valueByLeft("left4", Qt::CaseInsensitive) == "RIGHT4");
}

void HashTablesTestTest::biStrHash2()
{
    BiStrHash hash;
    hash.insert("left1", "right1");
    hash.insert("LEFT2", "right2");
    hash.insert("left3", "RIGHT3");
    hash.insert("LEFT4", "RIGHT4");
    hash.insert("LEFT5", "RIGHT5");
    hash.insert("LEFT6", "RIGHT6");
    hash.insert("LEFT7", "RIGHT7");
    hash.insert("left5", "x");
    hash.insert("y", "right6");

    QVERIFY(hash.count() == 7);

    hash.removeLeft("left2");
    hash.removeLeft("left3");
    hash.removeLeft("left4", Qt::CaseInsensitive);
    hash.removeRight("RIGHT1");
    hash.removeRight("right7", Qt::CaseInsensitive);

    QVERIFY(hash.count() == 4);
    QVERIFY(!hash.containsLeft("LEFT5"));
    QVERIFY(hash.valueByLeft("LEFT5", Qt::CaseInsensitive) == "x");
    QVERIFY(!hash.containsRight("RIGHT6"));
    QVERIFY(hash.valueByRight("RIGHT6", Qt::CaseInsensitive) == "y");
}

void HashTablesTestTest::biStrHash3()
{
    BiStrHash hash;
    hash.insert("left1", "right1");
    hash.insert("LEFT2", "RIGHT2");

    QVERIFY(hash.count() == 2);

    hash.insert("left2", "RIGHT1");

    QVERIFY(hash.count() == 1);
    QVERIFY(hash.valueByLeft("left2") == "RIGHT1");
    QVERIFY(hash.valueByRight("RIGHT1") == "left2");
    QVERIFY(hash.valueByLeft("LEFT2", Qt::CaseInsensitive) == "RIGHT1");
    QVERIFY(hash.valueByRight("right1", Qt::CaseInsensitive) == "left2");
}

void HashTablesTestTest::biHash1()
{
    BiHash<QString,QString> hash;
    hash.insert("left", "right");
    hash.insert("LEFT", "RIGHT");

    QVERIFY(hash.count() == 2);
    QVERIFY(hash.valueByLeft("left") == "right");
    QVERIFY(hash.valueByRight("RIGHT") == "LEFT");

    QVERIFY(hash.removeLeft("x") == 0);
    QVERIFY(hash.removeLeft("left") == 1);
    QVERIFY(hash.count() == 1);
    QVERIFY(hash.removeRight("RIGHT") == 1);
    QVERIFY(hash.count() == 0);
}

void HashTablesTestTest::biHash2()
{
    BiHash<QString,QString> hash;
    hash.insert("left", "right");
    hash.insert("LEFT", "RIGHT");

    QVERIFY(hash.count() == 2);

    hash.insert("left", "RIGHT");

    QVERIFY(hash.count() == 1);
    QVERIFY(hash.valueByLeft("left") == "RIGHT");
    QVERIFY(hash.takeRight("RIGHT") == "left");
    QVERIFY(hash.count() == 0);
}

QTEST_APPLESS_MAIN(HashTablesTestTest)

#include "tst_hashtablestesttest.moc"
