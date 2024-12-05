<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh-CN" sourcelanguage="en">
  <context>
    <name>AbstractDb</name>
    <message>
      <location filename="../db/abstractdb.cpp" line="352"/>
      <location filename="../db/abstractdb.cpp" line="369"/>
      <source>Cannot execute query on closed database.</source>
      <translation>无法在关闭的数据库上执行查询。</translation>
    </message>
    <message>
      <location filename="../db/abstractdb.cpp" line="709"/>
      <source>Error attaching database %1: %2</source>
      <translation>附加数据库 %1 时发生错误：%2</translation>
    </message>
    <message>
      <location filename="../db/abstractdb.cpp" line="958"/>
      <source>Failed to make full WAL checkpoint on database &apos;%1&apos;. Error returned from SQLite engine: %2</source>
      <translation>无法在数据库 &apos;%1&apos; 设置完整的 WAL 检查点。从 SQLite 引擎返回错误︰ %2</translation>
    </message>
  </context>
  <context>
    <name>ChainExecutor</name>
    <message>
      <location filename="../db/chainexecutor.cpp" line="37"/>
      <source>The database for executing queries was not defined.</source>
      <comment>chain executor</comment>
      <translation>没有定义执行查询的数据库。</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="44"/>
      <source>The database for executing queries was not open.</source>
      <comment>chain executor</comment>
      <translation>没有打开执行查询的数据库。</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="54"/>
      <source>Could not disable foreign keys in the database. Details: %1</source>
      <comment>chain executor</comment>
      <translation>未能禁用该数据库中的外键。详情：%1</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="62"/>
      <source>Could not start a database transaction. Details: %1</source>
      <comment>chain executor</comment>
      <translation>未能启动数据库事务。详情：%1</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="89"/>
      <source>Interrupted</source>
      <comment>chain executor</comment>
      <translation>中断</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="151"/>
      <source>Could not commit a database transaction. Details: %1</source>
      <comment>chain executor</comment>
      <translation>未能提交数据库事务。详情：%1</translation>
    </message>
  </context>
  <context>
    <name>CompletionHelper</name>
    <message>
      <location filename="../completionhelper.cpp" line="159"/>
      <source>New row reference</source>
      <translation>新的行引用</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="166"/>
      <source>Old row reference</source>
      <translation>旧的行引用</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="171"/>
      <source>New table name</source>
      <translation>新的表名</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="174"/>
      <source>New index name</source>
      <translation>新的索引名</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="177"/>
      <source>New view name</source>
      <translation>新的视图名</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="180"/>
      <source>New trigger name</source>
      <translation>新的触发器名</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="183"/>
      <source>Table or column alias</source>
      <translation>表或列别名</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="186"/>
      <source>transaction name</source>
      <translation>事务名</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="189"/>
      <source>New column name</source>
      <translation>新列名</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="192"/>
      <source>Column data type</source>
      <translation>列数据类型</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="195"/>
      <source>Constraint name</source>
      <translation>约束名</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="208"/>
      <source>Error message</source>
      <translation>错误信息</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="257"/>
      <source>Any word</source>
      <translation>任意单词</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="438"/>
      <source>Default database</source>
      <translation>默认数据库</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="439"/>
      <source>Temporary objects database</source>
      <translation>临时对象数据库</translation>
    </message>
  </context>
  <context>
    <name>ConfigImpl</name>
    <message>
      <location filename="../services/impl/configimpl.cpp" line="881"/>
      <source>Could not start database transaction for deleting SQL history, therefore it&apos;s not deleted.</source>
      <translation>删除 SQL 历史的数据库事务启动失败，因此未删除。</translation>
    </message>
    <message>
      <location filename="../services/impl/configimpl.cpp" line="888"/>
      <source>Could not commit database transaction for deleting SQL history, therefore it&apos;s not deleted.</source>
      <translation>删除 SQL 历史的数据库事务提交失败，因此未删除。</translation>
    </message>
  </context>
  <context>
    <name>DbManagerImpl</name>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="64"/>
      <source>Could not add database %1: %2</source>
      <translation>无法添加数据库 %1：%2</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="137"/>
      <source>Database %1 could not be updated, because of an error: %2</source>
      <translation>数据库 %1 没有被更新，由于错误：%2</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="353"/>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="382"/>
      <source>Database file doesn&apos;t exist.</source>
      <translation>数据库文件不存在。</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="355"/>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="384"/>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="603"/>
      <source>No supporting plugin loaded.</source>
      <translation>没有加载所需的插件。</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="521"/>
      <source>Database could not be initialized.</source>
      <translation>无法初始化数据库。</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="531"/>
      <source>No suitable database driver plugin found.</source>
      <translation>没有找到合适的数据库驱动插件。</translation>
    </message>
  </context>
  <context>
    <name>DbObjectOrganizer</name>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="372"/>
      <location filename="../dbobjectorganizer.cpp" line="403"/>
      <source>Error while creating table in target database: %1</source>
      <translation>在目标数据库中创建表时发生错误：%1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="372"/>
      <source>Could not parse table.</source>
      <translation>无法解析表。</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="417"/>
      <source>Database %1 could not be attached to database %2, so the data of table %3 will be copied with SQLiteStudio as a mediator. This method can be slow for huge tables, so please be patient.</source>
      <translation>数据库 %1 无法附加到数据库 %2，因此将使用 SQLiteStudio 做中间人来复制表 %3 的数据。此方法用于大型表格可能会很慢，请耐心等待。</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="441"/>
      <source>Error while copying data for table %1: %2</source>
      <translation>在从表 %1 中复制数据时发生错误：%2</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="460"/>
      <location filename="../dbobjectorganizer.cpp" line="467"/>
      <location filename="../dbobjectorganizer.cpp" line="494"/>
      <source>Error while copying data to table %1: %2</source>
      <translation>在向表 %1 中复制数据时发生错误：%2</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="516"/>
      <source>Error while dropping source view %1: %2
Tables, indexes, triggers and views copied to database %3 will remain.</source>
      <translation>在丢弃源视图 %1:%2 时出错
已拷贝至数据库 %3 的表格、索引、触发器和视图将被保留。</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="523"/>
      <source>Error while creating view in target database: %1</source>
      <translation>在目标数据库中创建视图时发生错误：%1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="528"/>
      <source>Error while creating index in target database: %1</source>
      <translation>在目标数据库中创建索引时发生错误：%1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="533"/>
      <source>Error while creating trigger in target database: %1</source>
      <translation>在目标数据库中创建触发器时发生错误：%1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="664"/>
      <location filename="../dbobjectorganizer.cpp" line="671"/>
      <location filename="../dbobjectorganizer.cpp" line="680"/>
      <source>Could not parse object &apos;%1&apos; in order to move or copy it.</source>
      <translation>无法解析对象 &apos;%1&apos; 用于移动或复制。</translation>
    </message>
  </context>
  <context>
    <name>DdlHistoryModel</name>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="66"/>
      <source>Database name</source>
      <comment>ddl history header</comment>
      <translation>数据库名</translation>
    </message>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="68"/>
      <source>Database file</source>
      <comment>ddl history header</comment>
      <translation>数据库文件</translation>
    </message>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="70"/>
      <source>Date of execution</source>
      <comment>ddl history header</comment>
      <translation>执行日期</translation>
    </message>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="72"/>
      <source>Changes</source>
      <comment>ddl history header</comment>
      <translation>影响</translation>
    </message>
  </context>
  <context>
    <name>ExportManager</name>
    <message>
      <location filename="../services/exportmanager.cpp" line="71"/>
      <source>Export plugin %1 doesn&apos;t support exporing query results.</source>
      <translation>导出插件 %1 不支持导出查询结果。</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="97"/>
      <source>Export plugin %1 doesn&apos;t support exporing tables.</source>
      <translation>导出插件 %1 不支持导出表。</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="121"/>
      <source>Export plugin %1 doesn&apos;t support exporing databases.</source>
      <translation>导出插件 %1 不支持导出数据库。</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="154"/>
      <source>Export format &apos;%1&apos; is not supported. Supported formats are: %2.</source>
      <translation>导出格式 &apos;%1&apos; 不受支持，受支持的格式是：%2。</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="218"/>
      <source>Export to the clipboard was successful.</source>
      <translation>成功导出到剪贴板。</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="222"/>
      <source>Export to the file &apos;%1&apos; was successful.</source>
      <translation>成功导出到文件 &apos;%1&apos;。</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="224"/>
      <source>Export was successful.</source>
      <translation>导出成功。</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="266"/>
      <source>Could not export to file %1. File cannot be open for writting.</source>
      <translation>无法导出到文件 %1。文件无法以写模式打开。</translation>
    </message>
  </context>
  <context>
    <name>ExportWorker</name>
    <message>
      <location filename="../exportworker.cpp" line="123"/>
      <source>Error while exporting query results: %1</source>
      <translation>导出查询结果时发生错误：%1</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="209"/>
      <source>Error while counting data column width to export from query results: %1</source>
      <translation>计算从查询结果来导出数据的列宽度时出错：%1</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="353"/>
      <location filename="../exportworker.cpp" line="411"/>
      <source>Could not parse %1 in order to export it. It will be excluded from the export output.</source>
      <translation>无法为导出解析 %1。因此它将不会被包含在导出输出中。</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="615"/>
      <source>Error while reading data to export from table %1: %2</source>
      <translation>从表 %1读取数据并导出时出错：%2</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="623"/>
      <source>Error while counting data to export from table %1: %2</source>
      <translation>计算从 %1: %2 表导出数据的数量时出错</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="639"/>
      <source>Error while counting data column width to export from table %1: %2</source>
      <translation>计算从 %1: %2 表导出数据的列宽度时出错</translation>
    </message>
  </context>
  <context>
    <name>FunctionManagerImpl</name>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="199"/>
      <source>Could not create scripting context, probably the plugin is not configured properly</source>
      <translation>无法创建脚本上下文，可能是插件配置不正确</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="292"/>
      <source>Invalid number of arguments to function &apos;%1&apos;. Expected %2, but got %3.</source>
      <translation>无效的参数个数。对于函数 %1，需要 %2 个，但得到 %3 个。</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="406"/>
      <source>No such function registered in SQLiteStudio: %1(%2)</source>
      <translation>SQLiteStudio 中没有注册该函数：%1(%2)</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="412"/>
      <source>Function %1(%2) was registered with language %3, but the plugin supporting that language is not currently loaded.</source>
      <translation>函数 %1(%2) 以 %3 语言注册，但是支持该语言的插件没有被正确加载。</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="430"/>
      <source>Invalid regular expression pattern: %1</source>
      <translation>无效的正则表达式模式：%1</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="449"/>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="482"/>
      <source>Could not open file %1 for reading: %2</source>
      <translation>无法以读模式打开文件 %1：%2</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="504"/>
      <source>Could not open file %1 for writting: %2</source>
      <translation>无法以写模式打开文件 %1：%2</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="524"/>
      <source>Error while writting to file %1: %2</source>
      <translation>写入文件 %1 时发生错误：%2</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="542"/>
      <source>Unsupported scripting language: %1</source>
      <translation>不支持的脚本语言：%1</translation>
    </message>
  </context>
  <context>
    <name>GenericExportPlugin</name>
    <message>
      <location filename="../plugins/genericexportplugin.cpp" line="20"/>
      <source>Could not initialize text codec for exporting. Using default codec: %1</source>
      <translation>无法为导出初始化文本编解码器。使用默认的编解码器：%1</translation>
    </message>
  </context>
  <context>
    <name>ImportManager</name>
    <message>
      <location filename="../services/importmanager.cpp" line="99"/>
      <source>Imported data to the table &apos;%1&apos; successfully. Number of imported rows: %2</source>
      <translation>成功向表 &apos;%1&apos; 导入数据。影响行数：%2</translation>
    </message>
  </context>
  <context>
    <name>ImportWorker</name>
    <message>
      <location filename="../importworker.cpp" line="24"/>
      <source>No columns provided by the import plugin.</source>
      <translation>导入插件没有提供列。</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="31"/>
      <source>Could not start transaction in order to import a data: %1</source>
      <translation>无法为导入数据开始事务：%1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="54"/>
      <source>Could not commit transaction for imported data: %1</source>
      <translation>无法为导入数据提交事务：%1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="101"/>
      <source>Table &apos;%1&apos; has less columns than there are columns in the data to be imported. Excessive data columns will be ignored.</source>
      <translation>表 &apos;%1&apos; 的列少于即将导入数据的列数。过多的数据列将被忽略。</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="106"/>
      <source>Table &apos;%1&apos; has more columns than there are columns in the data to be imported. Some columns in the table will be left empty.</source>
      <translation>表 &apos;%1&apos; 的列多于即将导入数据的列数。一些字段将被留空。</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="125"/>
      <source>Could not create table to import to: %1</source>
      <translation>未能创建导入所用的表：%1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="134"/>
      <location filename="../importworker.cpp" line="185"/>
      <location filename="../importworker.cpp" line="192"/>
      <source>Error while importing data: %1</source>
      <translation>导入数据时发生错误：%1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="134"/>
      <location filename="../importworker.cpp" line="192"/>
      <source>Interrupted.</source>
      <comment>import process status update</comment>
      <translation>中断。</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="180"/>
      <source>Could not import data row number %1. The row was ignored. Problem details: %2</source>
      <translation>无法导入行号 %1 的数据。该行将被忽略。问题详情：%2</translation>
    </message>
  </context>
  <context>
    <name>PluginManagerImpl</name>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="546"/>
      <source>Cannot load plugin %1, because it&apos;s in conflict with plugin %2.</source>
      <translation>无法加载插件 %1，因为它与这些插件冲突：%2。</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="557"/>
      <source>Cannot load plugin %1, because its dependency was not loaded: %2.</source>
      <translation>无法加载插件 %1，因为它依赖的这些插件没有被加载：%2。</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="566"/>
      <source>Cannot load plugin %1. Error details: %2</source>
      <translation>无法加载插件 %1。错误详情：%2</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="582"/>
      <source>Cannot load plugin %1 (error while initializing plugin).</source>
      <translation>无法加载插件 %1（初始化插件时发生错误）。</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="743"/>
      <source>min: %1</source>
      <comment>plugin dependency version</comment>
      <translation>最小：%1</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="744"/>
      <source>max: %1</source>
      <comment>plugin dependency version</comment>
      <translation>最大：%1</translation>
    </message>
  </context>
  <context>
    <name>PopulateConstant</name>
    <message>
      <location filename="../plugins/populateconstant.cpp" line="10"/>
      <source>Constant</source>
      <comment>populate constant plugin name</comment>
      <translation>常量</translation>
    </message>
  </context>
  <context>
    <name>PopulateConstantConfig</name>
    <message>
      <location filename="../plugins/populateconstant.ui" line="20"/>
      <source>Constant value:</source>
      <translation>常量值：</translation>
    </message>
  </context>
  <context>
    <name>PopulateDictionary</name>
    <message>
      <location filename="../plugins/populatedictionary.cpp" line="17"/>
      <source>Dictionary</source>
      <comment>dictionary populating plugin name</comment>
      <translation>字典</translation>
    </message>
  </context>
  <context>
    <name>PopulateDictionaryConfig</name>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="20"/>
      <source>Dictionary file</source>
      <translation>字典文件</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="29"/>
      <source>Pick dictionary file</source>
      <translation>选择字典文件</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="39"/>
      <source>Word separator</source>
      <translation>单词分隔符</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="45"/>
      <source>Whitespace</source>
      <translation>空白</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="58"/>
      <source>Line break</source>
      <translation>换行</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="74"/>
      <source>Method of using words</source>
      <translation>单词用法</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="80"/>
      <source>Ordered</source>
      <translation>有序</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="93"/>
      <source>Randomly</source>
      <translation>随机</translation>
    </message>
  </context>
  <context>
    <name>PopulateManager</name>
    <message>
      <location filename="../services/populatemanager.cpp" line="89"/>
      <source>Table &apos;%1&apos; populated successfully.</source>
      <translation>表 &apos;%1&apos; 填充成功。</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandom</name>
    <message>
      <location filename="../plugins/populaterandom.cpp" line="13"/>
      <source>Random number</source>
      <translation>随机数</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandomConfig</name>
    <message>
      <location filename="../plugins/populaterandom.ui" line="20"/>
      <source>Constant prefix</source>
      <translation>固定前缀</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="26"/>
      <source>No prefix</source>
      <translation>无前缀</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="39"/>
      <source>Minimum value</source>
      <translation>最小值</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="61"/>
      <source>Maximum value</source>
      <translation>最大值</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="86"/>
      <source>Constant suffix</source>
      <translation>固定后缀</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="92"/>
      <source>No suffix</source>
      <translation>无后缀</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandomText</name>
    <message>
      <location filename="../plugins/populaterandomtext.cpp" line="14"/>
      <source>Random text</source>
      <translation>随机文本</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandomTextConfig</name>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="20"/>
      <source>Use characters from common sets:</source>
      <translation>使用常用字符集合：</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="36"/>
      <source>Minimum length</source>
      <translation>最小长度</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="64"/>
      <source>Letters from a to z.</source>
      <translation>字母 a 到 z。</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="67"/>
      <source>Alpha</source>
      <translation>英文字母</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="77"/>
      <source>Numbers from 0 to 9.</source>
      <translation>数字 0 到 9。</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="80"/>
      <source>Numeric</source>
      <translation>数字</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="90"/>
      <source>A whitespace, a tab and a new line character.</source>
      <translation>空格、制表符和换行符。</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="93"/>
      <source>Whitespace</source>
      <translation>空白字符</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="103"/>
      <source>Includes all above and all others.</source>
      <translation>包括上述及其他所有。</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="106"/>
      <source>Binary</source>
      <translation>二进制</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="119"/>
      <source>Use characters from my custom set:</source>
      <translation>使用自定义字符集合：</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="132"/>
      <source>Maximum length</source>
      <translation>最大长度</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="160"/>
      <source>If you type some character multiple times, it&apos;s more likely to be used.</source>
      <translation>字符被输入的次数越多，被使用的概率越大。</translation>
    </message>
  </context>
  <context>
    <name>PopulateScript</name>
    <message>
      <location filename="../plugins/populatescript.cpp" line="34"/>
      <source>Script</source>
      <translation>脚本</translation>
    </message>
  </context>
  <context>
    <name>PopulateScriptConfig</name>
    <message>
      <location filename="../plugins/populatescript.ui" line="26"/>
      <source>Initialization code (optional)</source>
      <translation>初始化代码（可选）</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.ui" line="45"/>
      <source>Per step code</source>
      <translation>步进代码</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.ui" line="70"/>
      <source>Language</source>
      <translation>语言</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.ui" line="89"/>
      <source>Help</source>
      <translation>帮助</translation>
    </message>
  </context>
  <context>
    <name>PopulateSequence</name>
    <message>
      <location filename="../plugins/populatesequence.cpp" line="13"/>
      <source>Sequence</source>
      <translation>序列</translation>
    </message>
  </context>
  <context>
    <name>PopulateSequenceConfig</name>
    <message>
      <location filename="../plugins/populatesequence.ui" line="33"/>
      <source>Start value:</source>
      <translation>起始值：</translation>
    </message>
    <message>
      <location filename="../plugins/populatesequence.ui" line="56"/>
      <source>Step:</source>
      <translation>步进：</translation>
    </message>
  </context>
  <context>
    <name>PopulateWorker</name>
    <message>
      <location filename="../populateworker.cpp" line="23"/>
      <source>Could not start transaction in order to perform table populating. Error details: %1</source>
      <translation>未能启动执行表填充的事务。错误细节：%1</translation>
    </message>
    <message>
      <location filename="../populateworker.cpp" line="69"/>
      <source>Error while populating table: %1</source>
      <translation>表填充出错：%1</translation>
    </message>
    <message>
      <location filename="../populateworker.cpp" line="80"/>
      <source>Could not commit transaction after table populating. Error details: %1</source>
      <translation>未能提交表填充事务。错误细节：%1</translation>
    </message>
  </context>
  <context>
    <name>QObject</name>
    <message>
      <location filename="../common/utils.cpp" line="940"/>
      <source>Could not open file &apos;%1&apos; for reading: %2</source>
      <translation>无法以读模式打开文件 &apos;%1&apos;：%2</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="437"/>
      <source>Could not open database: %1</source>
      <translation>无法打开数据库：%1</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="1235"/>
      <source>Result set expired or no row available.</source>
      <translation>结果集过期或者无可用的行。</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="333"/>
      <location filename="../db/abstractdb3.h" line="337"/>
      <source>Could not load extension %1: %2</source>
      <translation>无法加载扩展 %1：%2</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="423"/>
      <source>Could not run WAL checkpoint: %1</source>
      <translation>无法运行 WAL 检查点： %1</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="461"/>
      <source>Could not close database: %1</source>
      <translation>无法关闭数据库：%1</translation>
    </message>
    <message>
      <location filename="../impl/dbattacherimpl.cpp" line="114"/>
      <source>Could not attach database %1: %2</source>
      <translation>无法附加数据库 %1：%2</translation>
    </message>
    <message>
      <location filename="../parser/parsercontext.cpp" line="108"/>
      <location filename="../parser/parsercontext.cpp" line="110"/>
      <source>Incomplete query.</source>
      <translation>不完整的查询。</translation>
    </message>
    <message>
      <location filename="../parser/sqlite3_parse.cpp" line="2526"/>
      <source>Parser stack overflow</source>
      <translation>解析堆栈溢出</translation>
    </message>
    <message>
      <location filename="../parser/sqlite3_parse.cpp" line="5937"/>
      <source>Syntax error</source>
      <translation>语法错误</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.cpp" line="32"/>
      <source>Could not open dictionary file %1 for reading.</source>
      <translation>无法以读模式打开字典文件 %1。</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.cpp" line="93"/>
      <source>Dictionary file must exist and be readable.</source>
      <translation>字典文件必须存在且可读。</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.cpp" line="54"/>
      <source>Maximum value cannot be less than minimum value.</source>
      <translation>最大值不能小于最小值。</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.cpp" line="79"/>
      <source>Maximum length cannot be less than minimum length.</source>
      <translation>最大长度不能小于最小长度。</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.cpp" line="90"/>
      <source>Custom character set cannot be empty.</source>
      <translation>自定义字符集合不能为空。</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="61"/>
      <source>Could not find plugin to support scripting language: %1</source>
      <translation>无法找到提供脚本语言支持的插件：%1</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="70"/>
      <source>Could not get evaluation context, probably the %1 scripting plugin is not configured properly</source>
      <translation>无法获取评估上下文，可能是 %1 脚本插件配置不正确</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="84"/>
      <source>Error while executing populating initial code: %1</source>
      <translation>执行填充初始化代码时出错：%1</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="106"/>
      <source>Error while executing populating code: %1</source>
      <translation>执行填充代码时出错：%1</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="138"/>
      <source>Select implementation language.</source>
      <translation>选择实现语言。</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="139"/>
      <source>Implementation code cannot be empty.</source>
      <translation>实现代码不得为空。</translation>
    </message>
    <message>
      <location filename="../selectresolver.cpp" line="372"/>
      <source>Could not resolve data source for column: %1</source>
      <translation>无法解析该列的数据源：%1</translation>
    </message>
    <message>
      <location filename="../selectresolver.cpp" line="444"/>
      <source>Could not resolve table for column &apos;%1&apos;.</source>
      <translation>无法解析表的列 &apos;%1&apos;。</translation>
    </message>
    <message>
      <location filename="../services/impl/configimpl.cpp" line="761"/>
      <source>Could not initialize configuration file. Any configuration changes and queries history will be lost after application restart. Unable to create a file at following locations: %1.</source>
      <translation>无法初始化配置文件。所有的配置更改和查询历史都将在应用程序重启时丢失。无法在下列位置创建文件：%1。</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="347"/>
      <source>General purpose</source>
      <comment>plugin category name</comment>
      <translation>一般用途</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="348"/>
      <source>Database support</source>
      <comment>plugin category name</comment>
      <translation>数据库支持</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="349"/>
      <source>Code formatter</source>
      <comment>plugin category name</comment>
      <translation>代码格式化</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="350"/>
      <source>Scripting languages</source>
      <comment>plugin category name</comment>
      <translation>脚本语言</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="352"/>
      <source>Exporting</source>
      <comment>plugin category name</comment>
      <translation>导出</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="353"/>
      <source>Importing</source>
      <comment>plugin category name</comment>
      <translation>导入</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="354"/>
      <source>Table populating</source>
      <comment>plugin category name</comment>
      <translation>数据表填充</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="161"/>
      <source>Table %1 is referencing table %2, but the foreign key definition will not be updated for new table definition due to problems while parsing DDL of the table %3.</source>
      <translation>表 %1 引用了表 %2，但由于解析表 %3 的 DDL 出现问题，不会为新的表定义更新该外键。</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="510"/>
      <source>All columns indexed by the index %1 are gone. The index will not be recreated after table modification.</source>
      <translation>索引 %1 涵盖的所有列索引已消失。表修改后该索引也不会被重新触发。</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="554"/>
      <source>There is problem with proper processing trigger %1. It may be not fully updated afterwards and will need your attention.</source>
      <translation>处理触发器 %1 时出现问题。它可能没有被充分更新，这需要您的注意。</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="569"/>
      <source>All columns covered by the trigger %1 are gone. The trigger will not be recreated after table modification.</source>
      <translation>触发器 %1 涵盖的所有列已消失。表修改后该触发器也不会被重新触发。</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="601"/>
      <source>Cannot not update trigger %1 according to table %2 modification.</source>
      <translation>无法根据表 %2 的修改更新触发器 %1。</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="620"/>
      <source>Cannot not update view %1 according to table %2 modifications.
The view will remain as it is.</source>
      <translation>无法根据表 %2 的修改更新视图 %1。
视图将保持原样不变。</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="782"/>
      <location filename="../tablemodifier.cpp" line="806"/>
      <location filename="../tablemodifier.cpp" line="825"/>
      <source>There is a problem with updating an %1 statement within %2 trigger. One of the %1 substatements which might be referring to table %3 cannot be properly modified. Manual update of the trigger may be necessary.</source>
      <translation>更新 %2 触发器内的一个 %1 语句时出现问题。%1 子句中引用的表 %3 可能无法被正确修改。必要时需要手动更新触发器。</translation>
    </message>
    <message>
      <location filename="../viewmodifier.cpp" line="24"/>
      <source>Could not parse DDL of the view to be created. Details: %1</source>
      <translation>无法解析要创建的视图的 DDL。详情：%1</translation>
    </message>
    <message>
      <location filename="../viewmodifier.cpp" line="33"/>
      <source>Parsed query is not CREATE VIEW. It&apos;s: %1</source>
      <translation>解析的查询不是 CREATE VIEW。它是：%1</translation>
    </message>
    <message>
      <location filename="../viewmodifier.cpp" line="81"/>
      <source>SQLiteStudio was unable to resolve columns returned by the new view, therefore it won&apos;t be able to tell which triggers might fail during the recreation process.</source>
      <translation>SQLiteStudio 无法解析新建视图时返回的列，因此无法告知哪些触发器可能在触发过程中失败。</translation>
    </message>
  </context>
  <context>
    <name>QueryExecutor</name>
    <message>
      <location filename="../db/queryexecutor.cpp" line="203"/>
      <source>Execution interrupted.</source>
      <translation>执行被中断。</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="244"/>
      <source>Database is not open.</source>
      <translation>数据库没有打开。</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="252"/>
      <source>Only one query can be executed simultaneously.</source>
      <translation>只能同时执行一个查询。</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="349"/>
      <location filename="../db/queryexecutor.cpp" line="595"/>
      <source>An error occured while executing the count(*) query, thus data paging will be disabled. Error details from the database: %1</source>
      <translation>执行 count(*) 查询时出错，因此将禁用数据分页。数据库错误详情：%1</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="514"/>
      <source>SQLiteStudio was unable to extract metadata from the query. Results won&apos;t be editable.</source>
      <translation>SQLiteStudio 无法从查询中提取元数据。结果将不可编辑。</translation>
    </message>
  </context>
  <context>
    <name>QueryExecutorSmartHints</name>
    <message>
      <location filename="../db/queryexecutorsteps/queryexecutorsmarthints.cpp" line="77"/>
      <source>Column %1 in table %2 is referencing column %3 in table %4, but these columns have different data types: %5 vs. %6. This may cause issues related to foreign key value matching.</source>
      <translation>表 %2 中的列 %1 引用表 %4 中的列 %3，但这些列的数据类型不同：%5 与 %6。这可能会导致与外键值匹配相关的问题。</translation>
    </message>
  </context>
  <context>
    <name>ScriptingQtDbProxy</name>
    <message>
      <location filename="../plugins/scriptingqtdbproxy.cpp" line="48"/>
      <source>No database available in current context, while called JavaScript&apos;s %1 command.</source>
      <translation>调用 JavaScript 的 %1 命令期间，当前上下文没有可用的数据库。</translation>
    </message>
    <message>
      <location filename="../plugins/scriptingqtdbproxy.cpp" line="65"/>
      <source>Error from %1: %2</source>
      <translation>来自 %1 的错误：%2</translation>
    </message>
  </context>
  <context>
    <name>SqlFileExecutor</name>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="56"/>
      <source>Could not execute SQL, because application has failed to start transaction: %1</source>
      <translation>应用程序启动事务失败，因此无法执行 SQL：%1</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="87"/>
      <source>Execution from file cancelled. Any queries executed so far have been rolled back.</source>
      <translation>从文件执行已取消。执行的所有查询已回滚。</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="103"/>
      <source>Could not open file &apos;%1&apos; for reading: %2</source>
      <translation>无法以读模式打开文件 &apos;%1&apos;：%2</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="150"/>
      <source>Could not execute SQL, because application has failed to commit the transaction: %1</source>
      <translation>应用程序提交事务失败，因此无法执行 SQL：%1</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="155"/>
      <source>Finished executing %1 queries in %2 seconds. %3 were not executed due to errors.</source>
      <translation>在 %2 秒内完成了 %1 个查询的执行。其中 %3 个由于错误未被执行。</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="161"/>
      <source>Finished executing %1 queries in %2 seconds.</source>
      <translation>在 %2 秒内完成了 %1 个查询的执行。</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="168"/>
      <source>Could not execute SQL due to error.</source>
      <translation>由于错误无法执行 SQL。</translation>
    </message>
  </context>
  <context>
    <name>SqlHistoryModel</name>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="32"/>
      <source>Database</source>
      <comment>sql history header</comment>
      <translation>数据库</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="34"/>
      <source>Execution date</source>
      <comment>sql history header</comment>
      <translation>执行日期</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="36"/>
      <source>Time spent</source>
      <comment>sql history header</comment>
      <translation>用时</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="38"/>
      <source>Rows affected</source>
      <comment>sql history header</comment>
      <translation>影响行数</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="40"/>
      <source>SQL</source>
      <comment>sql history header</comment>
      <translation>SQL</translation>
    </message>
  </context>
  <context>
    <name>T</name>
    <message>
      <location filename="../db/abstractdb3.h" line="864"/>
      <source>Registered default collation on demand, under name: %1</source>
      <translation>已根据需要注册默认排序规则，名称为：%1</translation>
    </message>
  </context>
  <context>
    <name>UpdateManager</name>
    <message>
      <location filename="../services/updatemanager.cpp" line="92"/>
      <source>Could not check for updates (%1).</source>
      <translation>检查更新失败 (%1)。</translation>
    </message>
  </context>
</TS>
