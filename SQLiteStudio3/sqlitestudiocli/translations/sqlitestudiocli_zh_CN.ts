<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh-CN" sourcelanguage="en">
  <context>
    <name>CLI</name>
    <message>
      <location filename="../cli.cpp" line="98"/>
      <source>Current database: %1</source>
      <translation>当前数据库：%1</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="100"/>
      <source>No current working database is set.</source>
      <translation>目前未设定操作的数据库。</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="102"/>
      <source>Type %1 for help</source>
      <translation>输入 %1 获取帮助</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="254"/>
      <source>Database passed in command line parameters (%1) was already on the list under name: %2</source>
      <translation>通过命令行参数传入的数据库（%1）已在列表中，名为：%2</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="262"/>
      <source>Could not add database %1 to list.</source>
      <translation>未将数据库“%1”添加到列表。</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="289"/>
      <source>closed</source>
      <translation>已关闭</translation>
    </message>
  </context>
  <context>
    <name>CliCommand</name>
    <message>
      <location filename="../commands/clicommand.cpp" line="107"/>
      <source>Usage: %1%2</source>
      <translation>用法：%1%2</translation>
    </message>
  </context>
  <context>
    <name>CliCommandAdd</name>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="9"/>
      <source>Could not add database %1 to list.</source>
      <translation>未将数据库“%1”添加到列表。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="14"/>
      <source>Database added: %1</source>
      <translation>已添加数据库：%1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="19"/>
      <source>adds new database to the list</source>
      <translation>添加新的数据库到列表</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="24"/>
      <source>Adds given database pointed by &lt;path&gt; with given &lt;name&gt; to list the databases list. The &lt;name&gt; is just a symbolic name that you can later refer to. Just pick any unique name. For list of databases already on the list use %1 command.</source>
      <translation>添加指定 &lt;路径&gt; 的数据库到数据库列表，用指定的 &lt;名称&gt;。&lt;名称&gt; 是您之后可以用来引用它的名称。选择一个不重复的名称。查阅已在数据库列表中的数据库，请用 %1 命令。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="34"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>名称</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="35"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>路径</translation>
    </message>
  </context>
  <context>
    <name>CliCommandCd</name>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="10"/>
      <source>Changed directory to: %1</source>
      <translation>目录已改为：%1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="12"/>
      <source>Could not change directory to: %1</source>
      <translation>未能切换到目录：%1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="17"/>
      <source>changes current working directory</source>
      <translation>更改当前的工作目录</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="22"/>
      <source>Very similar command to &apos;cd&apos; known from Unix systems and Windows. It requires a &lt;path&gt; argument to be passed, therefore calling %1 will always cause a change of the directory. To learn what&apos;s the current working directory use %2 command and to list contents of the current working directory use %3 command.</source>
      <translation>非常类似 Unix 和 Windows 系统中的 &apos;cd&apos; 命令。需要传入一个 &lt;路径&gt; 参数，然后调用 %1 将始终 cause a change of the directory. To learn what&apos;s the current working directory use %2 command and to list contents of the current working directory use %3 command.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="33"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>路径</translation>
    </message>
  </context>
  <context>
    <name>CliCommandClose</name>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="10"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>没有设定当前数据库时无法调用 %1。使用 %2 命令指定当前数据库，或者传递数据库名称到 %3。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="21"/>
      <location filename="../commands/clicommandclose.cpp" line="29"/>
      <source>Connection to database %1 closed.</source>
      <translation>数据库 %1 的连接已关闭。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="24"/>
      <source>No such database: %1. Use %2 to see list of known databases.</source>
      <translation>没有这样的数据库：%1。使用 %2 查看已知数据库列表。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="35"/>
      <source>closes given (or current) database</source>
      <translation>关闭指定的或当前的数据库</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="40"/>
      <source>Closes the database connection. If the database was already closed, nothing happens. If &lt;name&gt; is provided, it should be the name of the database to close (as printed by the %1 command). If &lt;name&gt; is not provided, then the current working database is closed (see help for %2 for details).</source>
      <translation>关闭数据库连接。如果数据库已经关闭，则不会发生任何操作。如果提供了 &lt;name&gt;，则应是要关闭的数据库名称（由 %1 命令打印）。如果未提供 &lt;name&gt;，则关闭当前工作数据库（详情请查看 %2 的帮助）。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="50"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>名称</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDbList</name>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="12"/>
      <source>No current working database defined.</source>
      <translation>目前未定义操作的数据库。</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="18"/>
      <source>Databases:</source>
      <translation>数据库：</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="23"/>
      <location filename="../commands/clicommanddblist.cpp" line="34"/>
      <source>Name</source>
      <comment>CLI db name column</comment>
      <translation>名称</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="31"/>
      <location filename="../commands/clicommanddblist.cpp" line="61"/>
      <source>Open</source>
      <comment>CLI connection state column</comment>
      <translation>打开</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="31"/>
      <location filename="../commands/clicommanddblist.cpp" line="61"/>
      <source>Closed</source>
      <comment>CLI connection state column</comment>
      <translation>关闭</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="32"/>
      <location filename="../commands/clicommanddblist.cpp" line="36"/>
      <source>Connection</source>
      <comment>CLI connection state column</comment>
      <translation>连接</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="38"/>
      <location filename="../commands/clicommanddblist.cpp" line="45"/>
      <source>Database file path</source>
      <translation>数据库文件路径</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="70"/>
      <source>prints list of registered databases</source>
      <translation>打印已注册数据库列表</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="75"/>
      <source>Prints list of databases registered in the SQLiteStudio. Each database on the list can be in open or closed state and %1 tells you that. The current working database (aka default database) is also marked on the list with &apos;*&apos; at the start of its name. See help for %2 command to learn about the default database.</source>
      <translation>列出在 SQLiteStudio 中注册的数据库的列表。. Each database on the list can be in open or closed state and %1 tells you that. The current working database (aka default database) is also marked on the list with &apos;*&apos; at the start of its name. See help for %2 command to learn about the default database.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDesc</name>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="15"/>
      <source>No working database is set.
Call %1 command to set working database.
Call %2 to see list of all databases.</source>
      <translation>没有设定操作的数据库。
调用 %1 命令操作的数据库。
调用 %2 查阅数据库列表。</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="26"/>
      <source>Database is not open.</source>
      <translation>数据库未被打开。</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="35"/>
      <source>Cannot find table named: %1</source>
      <translation>无法找到名为 %1 的表</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="52"/>
      <source>shows details about the table</source>
      <translation>显示一个表的详细信息</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="63"/>
      <source>table</source>
      <translation>表</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="70"/>
      <source>Table: %1</source>
      <translation>表：%1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="74"/>
      <source>Column name</source>
      <translation>字段名</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="76"/>
      <source>Data type</source>
      <translation>数据类型</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="80"/>
      <source>Constraints</source>
      <translation>约束条件</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="105"/>
      <source>Virtual table: %1</source>
      <translation>虚拟表：%1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="109"/>
      <source>Construction arguments:</source>
      <translation>构造参数：</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="114"/>
      <source>No construction arguments were passed for this virtual table.</source>
      <translation>没有为此虚拟表传递结构参数。</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDir</name>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="33"/>
      <source>lists directories and files in current working directory</source>
      <translation>列出当前工作目录中的目录与文件</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="38"/>
      <source>This is very similar to &apos;dir&apos; command known from Windows and &apos;ls&apos; command from Unix systems.

You can pass &lt;pattern&gt; with wildcard characters to filter output.</source>
      <translation>这非常类似 Windows 中的 &apos;dir&apos; 命令与 Unix 中的 &apos;ls&apos; 命令。

可以传入一个带有通配符的 &lt;模式&gt; 来过滤输出内容。</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="49"/>
      <source>pattern</source>
      <translation>模式</translation>
    </message>
  </context>
  <context>
    <name>CliCommandExit</name>
    <message>
      <location filename="../commands/clicommandexit.cpp" line="12"/>
      <source>quits the application</source>
      <translation>退出本程序</translation>
    </message>
    <message>
      <location filename="../commands/clicommandexit.cpp" line="17"/>
      <source>Quits the application. Settings are stored in configuration file and will be restored on next startup.</source>
      <translation>退出本程序。设置已被存储在配置文件并且会在下一次启动时恢复。</translation>
    </message>
  </context>
  <context>
    <name>CliCommandHelp</name>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="16"/>
      <source>shows this help message</source>
      <translation>显示这个帮助信息</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="21"/>
      <source>Use %1 to learn about certain commands supported by the command line interface (CLI) of the SQLiteStudio.
To see list of supported commands, type %2 without any arguments.

When passing &lt;command&gt; name, you can skip special prefix character (&apos;%3&apos;).

You can always execute any command with exactly single &apos;--help&apos; option to see help for that command. It&apos;s an alternative for typing: %1 &lt;command&gt;.</source>
      <translation>使用 %1 了解 SQLiteStudio 的命令行接口（CLI）所支持的特定命令。
输入 %2 不带任何参数来查看支持的命令列表。

传入 &lt;名称&gt; 名称时，您可以跳过特殊的前缀字符（&apos;%3&apos;）。

您可以为任何命令指定 &apos;--help&apos; 选项并执行来查看特定命令的帮助。另一种方法：%1 &lt;命令&gt;。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="33"/>
      <source>command</source>
      <comment>CLI command syntax</comment>
      <translation>命令</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="42"/>
      <source>No such command: %1</source>
      <translation>没有这个命令：%1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="43"/>
      <source>Type &apos;%1&apos; for list of available commands.</source>
      <translation>输入 &apos;%1&apos; 列出所有可用的命令。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="52"/>
      <source>Usage: %1%2</source>
      <translation>用法： %1%2</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="62"/>
      <source>Aliases: %1</source>
      <translation>别名：%1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandHistory</name>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="23"/>
      <source>Current history limit is set to: %1</source>
      <translation>当前历史记录限制为：%1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="39"/>
      <source>prints history or erases it</source>
      <translation>列出历史或擦除</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="44"/>
      <source>When no argument was passed, this command prints command line history. Every history entry is separated with a horizontal line, so multiline entries are easier to read.

When the -c or --clear option is passed, then the history gets erased.
When the -l or --limit option is passed, it sets the new history entries limit. It requires an additional argument saying how many entries do you want the history to be limited to.
Use -ql or --querylimit option to see the current limit value.</source>
      <translation>没有传入参数时，此命令列出命令行历史。每条历史以水平线隔开，以使多行单条更易阅读。

传入 -c 或 --clear 选项，历史记录将被清空擦除。
传入 -l 或 --limit 选项，设置历史记录条数限制。需要附上额外参数，指明将历史记录限制为最多多少条。
使用 -ql 或 --querylimit 选项，可查看当前的限制值。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="59"/>
      <source>number</source>
      <translation>数值</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="66"/>
      <source>Console history erased.</source>
      <translation>控制台历史已擦除。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="75"/>
      <source>Invalid number: %1</source>
      <translation>无效数值：%1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="80"/>
      <source>History limit set to %1</source>
      <translation>历史记录限制已设为 %1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandMode</name>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="9"/>
      <source>Current results printing mode: %1</source>
      <translation>当前结果打印模式：%1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="16"/>
      <source>Invalid results printing mode: %1</source>
      <translation>无效结果打印模式：%1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="21"/>
      <source>New results printing mode: %1</source>
      <translation>新结果打印模式：%1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="26"/>
      <source>tells or changes the query results format</source>
      <translation>询问或更改查询结果的格式</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="31"/>
      <source>When called without argument, tells the current output format for a query results. When the &lt;mode&gt; is passed, the mode is changed to the given one. Supported modes are:
- CLASSIC - columns are separated by a comma, not aligned,
- FIXED   - columns have equal and fixed width, they always fit into terminal window width, but the data in columns can be cut off,
- COLUMNS - like FIXED, but smarter (do not use with huge result sets, see details below),
- ROW     - each column from the row is displayed in new line, so the full data is displayed.

The CLASSIC mode is recommended if you want to see all the data, but you don&apos;t want to waste lines for each column. Each row will display full data for every column, but this also means, that columns will not be aligned to each other in next rows. The CLASSIC mode also doesn&apos;t respect the width of your terminal (console) window, so if values in columns are wider than the window, the row will be continued in next lines.

The FIXED mode is recommended if you want a readable output and you don&apos;t care about long data values. Columns will be aligned, making the output a nice table. The width of columns is calculated from width of the console window and a number of columns.

The COLUMNS mode is similar to FIXED mode, except it tries to be smart and make columns with shorter values more thin, while columns with longer values get more space. First to shrink are columns with longest headers (so the header names are to be cut off as first), then columns with the longest values are shrinked, up to the moment when all columns fit into terminal window.
ATTENTION! The COLUMNS mode reads all the results from the query at once in order to evaluate column widths, therefore it is dangerous to use this mode when working with huge result sets. Keep in mind that this mode will load entire result set into memory.

The ROW mode is recommended if you need to see whole values and you don&apos;t expect many rows to be displayed, because this mode displays a line of output per each column, so you&apos;ll get 10 lines for single row with 10 columns, then if you have 10 of such rows, you will get 100 lines of output (+1 extra line per each row, to separate rows from each other).</source>
      <translation>调用时不带参数，用于显示查询结果的当前输出格式。如果传递了 &lt;mode&gt; ，则会将模式更改为给定的模式。支持的模式有
- CLASSIC - 各列以逗号分隔，不对齐、
- FIXED（固定）- 列的宽度相等且固定，始终适合终端窗口的宽度，但列中的数据可以截断、
- COLUMNS - 与 FIXED 类似，但更智能（请勿用于庞大的结果集，详见下文）、
- ROW - 行中的每一列都以新行显示，因此可以显示完整的数据。

如果想查看所有数据，但又不想浪费每列的行数，建议使用经典模式。每一行将显示每一列的全部数据，但这也意味着下一行的列之间不会对齐。经典模式也不会考虑终端（控制台）窗口的宽度，因此如果列中的值比窗口宽，该行将在下一行继续显示。

如果希望输出结果清晰易读，且不在意数据值是否过长，建议使用固定模式。列将对齐，使输出成为漂亮的表格。列宽根据控制台窗口宽度和列数计算。

COLUMNS 模式与 FIXED 模式类似，但它会尽量聪明地让数值较短的列变得更细，而让数值较长的列获得更多空间。首先缩减的是标题最长的列（因此标题名称将首先被切掉），然后缩减数值最长的列，直至所有列都适合终端窗口。
注意 为了评估列宽，“列 ”模式会一次性读取查询的所有结果，因此在处理庞大的结果集时使用该模式很危险。请记住，该模式会将整个结果集加载到内存中。

如果需要查看全部数值，且不希望显示很多行，则推荐使用 ROW（行）模式，因为该模式下每列显示一行输出结果，因此 10 列的单行将显示 10 行输出结果，如果有 10 行这样的行，则将显示 100 行输出结果（每行+1 行额外输出结果，用于区分行与行）。</translation>
    </message>
  </context>
  <context>
    <name>CliCommandNullValue</name>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="9"/>
      <source>Current NULL representation string: %1</source>
      <translation>当前表示 NULL 的字符串：%1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="15"/>
      <source>tells or changes the NULL representation string</source>
      <translation>询问或更改表示 NULL 的字符串</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="20"/>
      <source>If no argument was passed, it tells what&apos;s the current NULL value representation (that is - what is printed in place of NULL values in query results). If the argument is given, then it&apos;s used as a new string to be used for NULL representation.</source>
      <translation>如果不传入任何参数，则会告知当前的 NULL 值表示方法（即查询结果中以什么代表 NULL 值）。如果提供了参数，则参数将作为新的代表 NULL 值的字符串。</translation>
    </message>
  </context>
  <context>
    <name>CliCommandOpen</name>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="12"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>没有设定当前数据库时无法调用 %1。使用 %2 命令指定当前数据库，或者传递数据库名称到 %3。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="29"/>
      <source>Could not add database %1 to list.</source>
      <translation>未能将数据库“%1”添加到列表。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="37"/>
      <source>File %1 doesn&apos;t exist in %2. Cannot open inexisting database with %3 command. To create a new database, use %4 command.</source>
      <translation>文件 %1 不存在于 %2。无法使用 %3 命令打开不存在的数据库。使用 %4 命令创建一个新数据库。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="61"/>
      <source>Database %1 has been open and set as the current working database.</source>
      <translation>已打开数据库 %1 并将其设为当前操作的数据库。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="66"/>
      <source>opens database connection</source>
      <translation>打开数据库连接</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="71"/>
      <source>Opens connection to the database. If no additional argument was passed, then the connection is open to the current default database (see help for %1 for details). However if an argument was passed, it can be either &lt;name&gt; of the registered database to open, or it can be &lt;path&gt; to the database file to open. In the second case, the &lt;path&gt; gets registered on the list with a generated name, but only for the period of current application session. After restarting application such database is not restored on the list.</source>
      <translation>打开到数据库的连接。如果不提供额外的参数，则打开到当前的默认数据库（详见 %1）的连接。如果提供一个参数，它可以是已注册的数据库的 &lt;name&gt;，也可以是要打开的数据库文件的 &lt;path&gt;。第二种情况下，&lt;path&gt; 将使用自动生成的名称临时注册到数据库列表，并在应用程序退出时从列表中消失。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="83"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>名称</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="83"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>路径</translation>
    </message>
  </context>
  <context>
    <name>CliCommandPwd</name>
    <message>
      <location filename="../commands/clicommandpwd.cpp" line="13"/>
      <source>prints the current working directory</source>
      <translation>列出当前的工作目录</translation>
    </message>
    <message>
      <location filename="../commands/clicommandpwd.cpp" line="18"/>
      <source>This is the same as &apos;pwd&apos; command on Unix systems and &apos;cd&apos; command without arguments on Windows. It prints current working directory. You can change the current working directory with %1 command and you can also list contents of the current working directory with %2 command.</source>
      <translation>这与 Unix 系统上的 &apos;pwd&apos; 命令以及 Windows 系统上没有参数的 &apos;cd&apos; 命令作用相同。将列出当前的工作目录。使用 %1 命令可以更改当前的工作目录，您也可以用 %2 命令列出当前工作目录的内容。</translation>
    </message>
  </context>
  <context>
    <name>CliCommandRemove</name>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="12"/>
      <source>No such database: %1</source>
      <translation>没有这样一个数据库：%1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="20"/>
      <source>Database removed: %1</source>
      <translation>数据库已移除：%1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="26"/>
      <source>New current database set:</source>
      <translation>新的当前数据库设为：</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="35"/>
      <source>removes database from the list</source>
      <translation>从列表中移除数据库</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="40"/>
      <source>Removes &lt;name&gt; database from the list of registered databases. If the database was not on the list (see %1 command), then error message is printed and nothing more happens.</source>
      <translation>从已注册数据库列表中移除名为 &lt;名称&gt; 的数据库。如果列表（见 %1 命令）中没有所指定的数据库 ，会给出错误消息。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="50"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>名称</translation>
    </message>
  </context>
  <context>
    <name>CliCommandSql</name>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="18"/>
      <source>No working database is set.
Call %1 command to set working database.
Call %2 to see list of all databases.</source>
      <translation>没有设定操作的数据库。
调用 %1 命令操作的数据库。
调用 %2 查阅数据库列表。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="29"/>
      <source>Database is not open.</source>
      <translation>数据库没有打开。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="64"/>
      <source>executes SQL query</source>
      <translation>执行 SQL 查询</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="69"/>
      <source>This command is executed every time you enter SQL query in command prompt. It executes the query on the current working database (see help for %1 for details). There&apos;s no sense in executing this command explicitly. Instead just type the SQL query in the command prompt, without any command prefixed.</source>
      <translation>您每次在命令行提示符中输入 SQL 查询时会执行此命令。它负责在当前操作的数据库（详见 %1）上执行查询。专门执行此命令没有任何意义。您可以在命令行提示符中直接输入 SQL 查询，无需添加命令前缀。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="85"/>
      <source>sql</source>
      <comment>CLI command syntax</comment>
      <translation>sql</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="134"/>
      <location filename="../commands/clicommandsql.cpp" line="176"/>
      <source>Too many columns to display in %1 mode.</source>
      <translation>在 %1 模式下有太多列需要显示。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="253"/>
      <source>Row %1</source>
      <translation>行 %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="403"/>
      <source>Query execution error: %1</source>
      <translation>查询执行错误：%1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandTables</name>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="15"/>
      <source>No such database: %1. Use %2 to see list of known databases.</source>
      <translation>没有这样一个数据库：%1。使用 %2 去查看已知的数据库列表。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="25"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>没有设置当前数据库时无法调用 %1。用 %2 命令指定当前数据库，或者传递数据库名称到 %3。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="32"/>
      <source>Database %1 is closed.</source>
      <translation>数据库 %1 已关闭。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="45"/>
      <location filename="../commands/clicommandtables.cpp" line="47"/>
      <source>Database</source>
      <translation>数据库</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="47"/>
      <source>Table</source>
      <translation>表</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="61"/>
      <source>prints list of tables in the database</source>
      <translation>列出数据库中的所有表</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="66"/>
      <source>Prints list of tables in given &lt;database&gt; or in the current working database. Note, that the &lt;database&gt; should be the name of the registered database (see %1). The output list includes all tables from any other databases attached to the queried database.
When the -s option is given, then system tables are also listed.</source>
      <translation>列出指定的 &lt;database&gt; 或当前操作的数据库的表。注意，&lt;database&gt; 应是已注册的数据库的名称（见 %1）。输出的列表同时包含已附加到被查询数据库的其他数据库的所有表。提供 -s 选项时，将同时列出系统表。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="77"/>
      <source>database</source>
      <comment>CLI command syntax</comment>
      <translation>数据库</translation>
    </message>
  </context>
  <context>
    <name>CliCommandTree</name>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="12"/>
      <source>No current working database is selected. Use %1 to define one and then run %2.</source>
      <translation>目前没有选择要操作的数据库。使用 %1 定义一个，然后运行 %2。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="54"/>
      <source>Tables</source>
      <translation>表</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="58"/>
      <source>Views</source>
      <translation>视图</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="83"/>
      <source>Columns</source>
      <translation>列</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="88"/>
      <source>Indexes</source>
      <translation>索引</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="92"/>
      <location filename="../commands/clicommandtree.cpp" line="113"/>
      <source>Triggers</source>
      <translation>触发器</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="132"/>
      <source>prints all objects in the database as a tree</source>
      <translation>将数据库中的所有对象列为一个树</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="137"/>
      <source>Prints all objects (tables, indexes, triggers and views) that are in the database as a tree. The tree is very similar to the one that you can see in GUI client of the SQLiteStudio.
When -c option is given, then also columns will be listed under each table.
When -s option is given, then also system objects will be printed (sqlite_* tables, autoincrement indexes, etc).
The database argument is optional and if provided, then only given database will be printed. This is not a registered database name, but instead it&apos;s an internal SQLite database name, like &apos;main&apos;, &apos;temp&apos;, or any attached database name. To print tree for other registered database, call %1 first to switch the working database, and then use %2 command.</source>
      <translation>列出数据库中的所有对象（表、索引、触发器和视图）为一个树。此树非常类似您在 SQLiteStudio 的图形用户界面（GUI）版本中看到的效果。
提供 -c 选项时，会同时在每个表下列出它的列。
提供 -s 选项时，会同时列出系统对象（sqlite_* 表、自动增量索引等）。
数据库参数为可选，如果提供则仅列出所给出的数据库。这不是数据库在列表中注册的名称，而是其在 SQLIte 数据库内部的名称，例如 &apos;main&apos;、&apos;temp&apos; 等。如果要列出列表中注册的其他数据库，先调用 %1 切换当前操作的数据库，然后再使用 %2 命令。</translation>
    </message>
  </context>
  <context>
    <name>CliCommandUse</name>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="13"/>
      <source>No current database selected.</source>
      <translation>目前没有选择数据库。</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="16"/>
      <location filename="../commands/clicommanduse.cpp" line="30"/>
      <source>Current database: %1</source>
      <translation>当前数据库：%1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="23"/>
      <source>No such database: %1</source>
      <translation>没有这样一个数据库：%1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="35"/>
      <source>changes default working database</source>
      <translation>更改默认操作的数据库</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="40"/>
      <source>Changes current working database to &lt;name&gt;. If the &lt;name&gt; database is not registered in the application, then the error message is printed and no change is made.

What is current working database?
When you type a SQL query to be executed, it is executed on the default database, which is also known as the current working database. Most of database-related commands can also work using default database, if no database was provided in their arguments. The current database is always identified by command line prompt. The default database is always defined (unless there is no database on the list at all).

The default database can be selected in various ways:
- using %1 command,
- by passing database file name to the application startup parameters,
- by passing registered database name to the application startup parameters,
- by restoring previously selected default database from saved configuration,
- or when default database was not selected by any of the above, then first database from the registered databases list becomes the default one.</source>
      <translation>更改当前操作的数据库至 &lt;name&gt;。如果 &lt;name&gt; 数据库没有在本程序中注册，将给出错误消息并且什么也不做。

什么是当前操作的数据库。
当您输入一条 SQL 查询以期执行时，它会在默认数据库上执行，这也被称为当前操作（或称作业）的数据库。大多数与数据库相关的命令也在没有额外指明时使用默认数据库。当前的数据库会始终在命令行中标明。会始终有一个默认数据库，除非数据库列表为空。

有多种方式选择默认数据库。
- 使用 %1 命令；
- 本程序启动时将数据库的文件名作为启动参数传入；
- 本程序启动时将已注册的数据库名称；
- 从已保存的配置文件还原之前选择的默认数据库；
- 未通过以上任何方式选择默认数据库时，注册的数据库列表中的第一个数据库将作为默认数据库。</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="63"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>名称</translation>
    </message>
  </context>
  <context>
    <name>QObject</name>
    <message>
      <location filename="../clicommandsyntax.cpp" line="155"/>
      <source>Insufficient number of arguments.</source>
      <translation>参数数量不足。</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="325"/>
      <source>Too many arguments.</source>
      <translation>参数过多。</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="347"/>
      <source>Invalid argument value: %1.
Expected one of: %2</source>
      <translation>无效参数值：%1。
预期可能是：%2</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="383"/>
      <source>Unknown option: %1</source>
      <comment>CLI command syntax</comment>
      <translation>未知选项：%1</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="394"/>
      <source>Option %1 requires an argument.</source>
      <comment>CLI command syntax</comment>
      <translation>选项 %1 要求一个参数。</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="31"/>
      <source>string</source>
      <comment>CLI command syntax</comment>
      <translation>字符串</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="28"/>
      <source>Command line interface to SQLiteStudio, a SQLite manager.</source>
      <translation>SQLite 管理工具 SQLiteStudio 的命令行接口。</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="32"/>
      <source>Enables debug messages on standard error output.</source>
      <translation>启用调试消息输出到标准错误输出。</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="33"/>
      <source>Enables Lemon parser debug messages for SQL code assistant.</source>
      <translation>启用 SQL 代码助手的 Lemon 解析器调试消息。</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="34"/>
      <source>Lists plugins installed in the SQLiteStudio and quits.</source>
      <translation>列出 SQLiteStudio 中已安装的插件然后退出。</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="36"/>
      <source>Executes provided SQL file (including all rich features of SQLiteStudio&apos;s query executor) on the specified database file and quits. The database parameter becomes mandatory if this option is used.</source>
      <translation>在指定的数据库文件上执行提供的 SQL 文件（包括 SQLiteStudio&apos;s 查询执行器的所有丰富功能）并退出。如果使用此选项，数据库参数将成为必填项。</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="39"/>
      <source>SQL file</source>
      <translation>SQL 文件</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="40"/>
      <source>Character encoding to use when reading SQL file (-e option). Use -cl to list available codecs. Defaults to %1.</source>
      <translation>读取 SQL 文件时使用的字符编码（-e 选项）。使用 -cl 列出可用的编解码器。默认为 %1。</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="43"/>
      <source>codec</source>
      <translation>解码器</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="44"/>
      <source>Lists available codecs to be used with -c option and quits.</source>
      <translation>列出与 -c 选项一起使用的可用编解码器，然后退出。</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="46"/>
      <source>When used together with -e option, the execution will not stop on an error, but rather continue until the end, ignoring errors.</source>
      <translation>与 -e 选项一起使用时，执行不会因出错而停止，而是继续到结束，忽略错误。</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="57"/>
      <source>file</source>
      <translation>文件</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="57"/>
      <source>Database file to open</source>
      <translation>要打开的数据库文件</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="78"/>
      <source>Invalid codec: %1. Use -cl option to list available codecs.</source>
      <translation>无效编解码器：%1。使用 -cl 选项列出可用的编解码器。</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="108"/>
      <source>Database file argument is mandatory when executing SQL file.</source>
      <translation>在执行 SQL 文件时，数据库文件参数是强制性的。</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="114"/>
      <source>Could not open specified database for executing SQL file. You may try using -d option to find out more details.</source>
      <translation>无法打开指定数据库以执行 SQL 文件。您可以尝试使用 -d 选项了解更多详情。</translation>
    </message>
  </context>
</TS>
