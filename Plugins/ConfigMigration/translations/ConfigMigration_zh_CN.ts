<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh-CN" sourcelanguage="en">
  <context>
    <name>ConfigMigration</name>
    <message>
      <location filename="../configmigration.cpp" line="36"/>
      <source>A configuration from old SQLiteStudio 2.x.x has been detected. Would you like to migrate old settings into the current version? &lt;a href=&quot;%1&quot;&gt;Click here to do that&lt;/a&gt;.</source>
      <translation>检测到旧的 SQLiteStudio 2.x.x 的配置。 你想要将旧的设置迁移到当前版本吗？ &lt;a href=&quot;%1&quot;&gt;点击这里进行迁移&lt;/a&gt;。</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="139"/>
      <source>Bug reports history (%1)</source>
      <translation>错误报告历史 (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="148"/>
      <source>Database list (%1)</source>
      <translation>数据库列表 (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="157"/>
      <source>Custom SQL functions (%1)</source>
      <translation>自定义 SQL 函数 (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="166"/>
      <source>SQL queries history (%1)</source>
      <translation>SQL 查询历史 (%1)</translation>
    </message>
  </context>
  <context>
    <name>ConfigMigrationWizard</name>
    <message>
      <location filename="../configmigrationwizard.ui" line="14"/>
      <source>Configuration migration</source>
      <translation>配置迁移</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="24"/>
      <source>Items to migrate</source>
      <translation>要迁移的项</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="27"/>
      <source>This is a list of items found in the old configuration file, which can be migrated into the current configuration.</source>
      <translation>这是在旧的配置文件中找到的项列表，这些项可以迁移到当前配置。</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="58"/>
      <source>Options</source>
      <translation>选项</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="64"/>
      <source>Put imported databases into separate group</source>
      <translation>将导入的数据库置于单独的组</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="76"/>
      <source>Group name</source>
      <translation>组名称</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="62"/>
      <source>Enter a non-empty name.</source>
      <translation>请输入一个非空的名称。</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="70"/>
      <source>Top level group named &apos;%1&apos; already exists. Enter a group name that does not exist yet.</source>
      <translation>名为 &apos;%1&apos; 的顶级组已存在。请输入未被占用的组名称。</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="104"/>
      <source>Could not open old configuration file in order to migrate settings from it.</source>
      <translation>无法打开旧的配置文件以迁移它的设置。</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="112"/>
      <source>Could not open current configuration file in order to migrate settings from old configuration file.</source>
      <translation>无法打开当前的配置文件以迁移旧的设置。</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="121"/>
      <source>Could not commit migrated data into new configuration file: %1</source>
      <translation>无法将迁移的数据提交到新的配置文件： %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="165"/>
      <source>Could not read bug reports history from old configuration file in order to migrate it: %1</source>
      <translation>无法从旧的配置文件读取错误报告历史记录来进行迁移： %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="182"/>
      <source>Could not insert a bug reports history entry into new configuration file: %1</source>
      <translation>无法将错误报告历史记录插入到新的配置文件： %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="203"/>
      <source>Could not read database list from old configuration file in order to migrate it: %1</source>
      <translation>无法从旧的配置文件读取数据库列表来进行迁移： %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="217"/>
      <source>Could not query for available order for containing group in new configuration file in order to migrate the database list: %1</source>
      <translation>无法在新配置文件中查询包含组的可用顺序以迁移数据库列表: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="228"/>
      <source>Could not create containing group in new configuration file in order to migrate the database list: %1</source>
      <translation>无法在新配置文件中创建包含组以迁移数据库列表: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="249"/>
      <source>Could not insert a database entry into new configuration file: %1</source>
      <translation>无法将数据库条目插入到新的配置文件：%1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="261"/>
      <source>Could not query for available order for next database in new configuration file in order to migrate the database list: %1</source>
      <translation>无法在新配置文件中查询下一个数据库的可用顺序以迁移数据库列表:%1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="272"/>
      <source>Could not create group referencing the database in new configuration file: %1</source>
      <translation>未能在新的配置文件中创建引用该数据库的组：%1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="290"/>
      <source>Could not read function list from old configuration file in order to migrate it: %1</source>
      <translation>无法从旧配置文件读取函数列表来进行迁移： %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="325"/>
      <source>Could not read SQL queries history from old configuration file in order to migrate it: %1</source>
      <translation>无法从旧配置文件读取 SQL 查询历史来进行迁移： %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="332"/>
      <source>Could not read next ID for SQL queries history in new configuration file: %1</source>
      <translation>无法在新配置文件中读取 SQL 查询历史的下一个ID： %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="348"/>
      <source>Could not insert SQL history entry into new configuration file: %1</source>
      <translation>无法将 SQL 历史条目插入到新的配置文件： %1</translation>
    </message>
  </context>
</TS>
