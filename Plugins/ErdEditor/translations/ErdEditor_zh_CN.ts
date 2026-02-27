<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh-CN" sourcelanguage="en">
<context>
    <name>ErdChangeRegistryDialog</name>
    <message>
        <location filename="../changes/erdchangeregistrydialog.ui" line="14"/>
        <source>Pending changes registry</source>
        <translation>待处理变更注册表</translation>
    </message>
    <message>
        <location filename="../changes/erdchangeregistrydialog.ui" line="65"/>
        <source>Pending changes</source>
        <translation>待处理的更改</translation>
    </message>
    <message>
        <location filename="../changes/erdchangeregistrydialog.ui" line="105"/>
        <source>DDL preview</source>
        <translation>DDL 预览</translation>
    </message>
    <message>
        <location filename="../changes/erdchangeregistrydialog.ui" line="143"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Shows the changes as they will be committed. Redundant or mutually cancelling steps are merged or removed from the list.&lt;br/&gt;When disabled, all individual undo and redo steps are shown.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;显示将要提交的更改。冗余或相互抵消的步骤将被合并或从列表中删除。&lt;br/&gt;禁用后，将显示所有单独的撤销和重做步骤。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../changes/erdchangeregistrydialog.ui" line="146"/>
        <source>Show effective changes only</source>
        <translation>仅显示有效变更</translation>
    </message>
    <message>
        <location filename="../changes/erdchangeregistrydialog.ui" line="153"/>
        <source>&lt;html&gt;&lt;body&gt;Shows changes that do not modify the database schema, but only affect the diagram (for example, table position changes). &lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;body&gt;显示不会修改数据库架构，而只会影响图表的更改（例如，表位置更改）。 &lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../changes/erdchangeregistrydialog.ui" line="156"/>
        <source>Show non-schema changes</source>
        <translation>显示非方案变更</translation>
    </message>
    <message>
        <location filename="../changes/erdchangeregistrydialog.cpp" line="58"/>
        <source>-- This is a change applied only to the diagram. It has no database schema efects.</source>
        <comment>ERD editor</comment>
        <translation>— 此更改仅应用于图表，不会影响数据库架构。</translation>
    </message>
</context>
<context>
    <name>ErdConfig</name>
    <message>
        <location filename="../erdconfig.ui" line="14"/>
        <source>Form</source>
        <translation>表单</translation>
    </message>
    <message>
        <location filename="../erdconfig.ui" line="33"/>
        <source>Maximum number of tables for ERD editor</source>
        <translation>ERD 编辑器的最大表格数量</translation>
    </message>
    <message>
        <location filename="../erdconfig.ui" line="40"/>
        <source>Setting this value too high may cause the application to slow down or become unresponsive when opening the ERD editor.</source>
        <translation>将此值设置得过高可能会导致应用程序在打开 ERD 编辑器时运行缓慢或无响应。</translation>
    </message>
    <message>
        <location filename="../erdconfig.ui" line="49"/>
        <source>Erd.MaxTableLimit</source>
        <translation>Erd.MaxTableLimit</translation>
    </message>
    <message>
        <location filename="../erdconfig.ui" line="56"/>
        <source>Starts panning as soon as the Space key is pressed, without requiring a mouse button press.</source>
        <translation>按下空格键即可开始平移，无需按鼠标按钮。</translation>
    </message>
    <message>
        <location filename="../erdconfig.ui" line="59"/>
        <source>Pan view with Space only</source>
        <translation>仅使用空格键的平移视图</translation>
    </message>
    <message>
        <location filename="../erdconfig.ui" line="62"/>
        <source>Erd.DragBySpace</source>
        <translation>Erd.DragBySpace</translation>
    </message>
</context>
<context>
    <name>ErdConnectionPanel</name>
    <message>
        <location filename="../panel/erdconnectionpanel.ui" line="14"/>
        <source>Form</source>
        <translation>表单</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.ui" line="59"/>
        <source>Foreign key properties</source>
        <translation>外键属性</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.ui" line="83"/>
        <source>Composite relation (multiple columns)</source>
        <translation>复合关系（多列）</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.ui" line="137"/>
        <source>Referencing (child) table:</source>
        <translation>引用（子）表：</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.ui" line="156"/>
        <source>Referencing (child) column:</source>
        <translation>引用（子）列：</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.cpp" line="51"/>
        <source>ERD side panel for relation between tables &quot;%1&quot; and &quot;%2&quot; has uncommitted modifications.</source>
        <translation>表 &quot;%1&quot; 和 &quot;%2&quot; 之间关系的 ERD 侧面板有未提交的修改。</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.cpp" line="57"/>
        <source>Apply changes to diagram</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Apply changes to diagram</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.cpp" line="58"/>
        <source>Abort changes</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Abort changes</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.cpp" line="240"/>
        <source>Modify relationship between &quot;%1&quot; and &quot;%2&quot;.</source>
        <translation type="unfinished">Modify relationship between &quot;%1&quot; and &quot;%2&quot;.</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.cpp" line="242"/>
        <source>Modify relationship between &quot;%1&quot; and &quot;%2&quot; - change target to &quot;%3&quot;.</source>
        <translation type="unfinished">Modify relationship between &quot;%1&quot; and &quot;%2&quot; - change target to &quot;%3&quot;.</translation>
    </message>
    <message>
        <location filename="../panel/erdconnectionpanel.cpp" line="252"/>
        <source>Failed to execute DDL required for relation modification. Details: %1</source>
        <translation type="unfinished">Failed to execute DDL required for relation modification. Details: %1</translation>
    </message>
</context>
<context>
    <name>ErdEditorPlugin</name>
    <message>
        <location filename="../erdeditorplugin.cpp" line="20"/>
        <source>Open ERD editor</source>
        <translation type="unfinished">Open ERD editor</translation>
    </message>
    <message>
        <location filename="../erdeditorplugin.cpp" line="74"/>
        <source>ERD editor cannot open because the database contains %1 tables, exceeding the configured limit of %2 tables. You can increase this limit in the settings, but higher values may slow down or freeze the application.</source>
        <translation type="unfinished">ERD editor cannot open because the database contains %1 tables, exceeding the configured limit of %2 tables. You can increase this limit in the settings, but higher values may slow down or freeze the application.</translation>
    </message>
</context>
<context>
    <name>ErdScene</name>
    <message>
        <location filename="../scene/erdscene.cpp" line="530"/>
        <source>Delete multiple diagram elements.</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Delete multiple diagram elements.</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="547"/>
        <source>Failed to execute the undo DDL. Details: %1</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Failed to execute the undo DDL. Details: %1</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="569"/>
        <source>Failed to execute the redo DDL. Details: %1</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Failed to execute the redo DDL. Details: %1</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="596"/>
        <source>Failed to execute DDL required for table deletion. Details: %1</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Failed to execute DDL required for table deletion. Details: %1</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="607"/>
        <source>Delete foreign key between &quot;%1&quot; and &quot;%2&quot;.</source>
        <translation type="unfinished">Delete foreign key between &quot;%1&quot; and &quot;%2&quot;.</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="616"/>
        <source>Failed to execute DDL required for foreign key deletion. Details: %1</source>
        <translation type="unfinished">Failed to execute DDL required for foreign key deletion. Details: %1</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="824"/>
        <source>Arrange entities</source>
        <translation type="unfinished">Arrange entities</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="825"/>
        <source>Are you sure you want to automatically arrange the entities on the diagram? This action will overwrite the current layout, and any manual adjustments will be lost.</source>
        <translation type="unfinished">Are you sure you want to automatically arrange the entities on the diagram? This action will overwrite the current layout, and any manual adjustments will be lost.</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="845"/>
        <source>Change color of table &quot;%1&quot; to %2.</source>
        <translation type="unfinished">Change color of table &quot;%1&quot; to %2.</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="851"/>
        <source>Change color of multiple tables.</source>
        <translation type="unfinished">Change color of multiple tables.</translation>
    </message>
    <message>
        <location filename="../scene/erdscene.cpp" line="696"/>
        <source>Apply diagram layout</source>
        <translation type="unfinished">Apply diagram layout</translation>
    </message>
</context>
<context>
    <name>ErdTableWindow</name>
    <message>
        <location filename="../panel/erdtablewindow.cpp" line="30"/>
        <source>Apply changes to diagram</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Apply changes to diagram</translation>
    </message>
    <message>
        <location filename="../panel/erdtablewindow.cpp" line="31"/>
        <source>Abort changes</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Abort changes</translation>
    </message>
    <message>
        <location filename="../panel/erdtablewindow.cpp" line="55"/>
        <source>ERD side panel for table &quot;%1&quot; has uncommitted modifications.</source>
        <translation type="unfinished">ERD side panel for table &quot;%1&quot; has uncommitted modifications.</translation>
    </message>
    <message>
        <location filename="../panel/erdtablewindow.cpp" line="112"/>
        <source>Invalid table changes</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Invalid table changes</translation>
    </message>
    <message>
        <location filename="../panel/erdtablewindow.cpp" line="114"/>
        <source>&lt;b&gt;The table contains invalid changes&lt;/b&gt;</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">&lt;b&gt;The table contains invalid changes&lt;/b&gt;</translation>
    </message>
    <message>
        <location filename="../panel/erdtablewindow.cpp" line="115"/>
        <source>Some of the changes you made cannot be applied because they contain errors.&lt;br&gt;&lt;br&gt;&lt;b&gt;Errors:&lt;/b&gt;&lt;br&gt;&lt;code&gt;%1&lt;/code&gt;&lt;br&gt;&lt;br&gt;You can &lt;b&gt;return to editing&lt;/b&gt; and fix the problems, or &lt;b&gt;discard your changes&lt;/b&gt; and restore the previous state of the table.</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Some of the changes you made cannot be applied because they contain errors.&lt;br&gt;&lt;br&gt;&lt;b&gt;Errors:&lt;/b&gt;&lt;br&gt;&lt;code&gt;%1&lt;/code&gt;&lt;br&gt;&lt;br&gt;You can &lt;b&gt;return to editing&lt;/b&gt; and fix the problems, or &lt;b&gt;discard your changes&lt;/b&gt; and restore the previous state of the table.</translation>
    </message>
    <message>
        <location filename="../panel/erdtablewindow.cpp" line="124"/>
        <source>Fix errors</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Fix errors</translation>
    </message>
</context>
<context>
    <name>ErdView</name>
    <message>
        <location filename="../scene/erdview.cpp" line="323"/>
        <source>Cannot edit compound foreign keys this way. Such connections have to be edited through the side panel.</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Cannot edit compound foreign keys this way. Such connections have to be edited through the side panel.</translation>
    </message>
    <message>
        <location filename="../scene/erdview.cpp" line="633"/>
        <source>Move table &quot;%1&quot;</source>
        <translation type="unfinished">Move table &quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../scene/erdview.cpp" line="639"/>
        <source>Move tables: %1</source>
        <translation type="unfinished">Move tables: %1</translation>
    </message>
</context>
<context>
    <name>ErdWindow</name>
    <message>
        <location filename="../erdwindow.ui" line="14"/>
        <source>Form</source>
        <translation type="unfinished">Form</translation>
    </message>
    <message>
        <location filename="../erdwindow.ui" line="86"/>
        <source>Select an item to edit its properties</source>
        <translation type="unfinished">Select an item to edit its properties</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="164"/>
        <source>Cancels ongoing action</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Cancels ongoing action</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="165"/>
        <source>Create a table</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Create a table</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="167"/>
        <location filename="../erdwindow.cpp" line="644"/>
        <source>Reload schema</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Reload schema</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="168"/>
        <source>Commit all pending changes</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Commit all pending changes</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="169"/>
        <source>Revert diagram to initial state</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Revert diagram to initial state</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="171"/>
        <source>Undo</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Undo</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="172"/>
        <source>Redo</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Redo</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="188"/>
        <source>Create a table</source>
        <translation type="unfinished">Create a table</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="201"/>
        <source>Select all</source>
        <translation type="unfinished">Select all</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="310"/>
        <source>Filter items</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Filter items</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="311"/>
        <source>Items that don’t match the filter will be dimmed.</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Items that don’t match the filter will be dimmed.</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="447"/>
        <source>table name</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">table name</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="449"/>
        <source>column name</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">column name</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="569"/>
        <source>All changes have been successfully applied to the database.</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">All changes have been successfully applied to the database.</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="575"/>
        <source>The changes were successfully committed. No modifications to the database schema were required.</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">The changes were successfully committed. No modifications to the database schema were required.</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="583"/>
        <source>Failed to apply changes to the database. Details: %1</source>
        <comment>ERD editor</comment>
        <translation type="unfinished">Failed to apply changes to the database. Details: %1</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="645"/>
        <source>This action will discard all your pending changes and reload the diagram from the current database schema. The undo/redo history will be cleared. Do you want to proceed?</source>
        <translation type="unfinished">This action will discard all your pending changes and reload the diagram from the current database schema. The undo/redo history will be cleared. Do you want to proceed?</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="826"/>
        <source>ERD window &quot;%1&quot; has uncommitted changes.</source>
        <translation type="unfinished">ERD window &quot;%1&quot; has uncommitted changes.</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="1138"/>
        <source>ERD editor (%1)</source>
        <translation type="unfinished">ERD editor (%1)</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="1140"/>
        <source>ERD editor</source>
        <translation type="unfinished">ERD editor</translation>
    </message>
    <message>
        <location filename="../erdwindow.ui" line="114"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;- Hold the Spacebar and drag with the mouse to pan the diagram freely without selecting any items.&lt;/p&gt;&lt;p&gt;- Use the mouse wheel to zoom in and out.&lt;/p&gt;&lt;p&gt;- Deselect the table (or click Commit in the side panel toolbar) to apply the side panel changes to the diagram.&lt;/p&gt;&lt;p&gt;- Press Esc (or click Rollback in the side panel toolbar) to discard the side panel changes.&lt;/p&gt;&lt;p&gt;- Double-Click on a table name or column to edit the name inline.&lt;/p&gt;&lt;p&gt;- Shift-Double-Click on a column to edit column details (datatype, constraints).&lt;/p&gt;&lt;p&gt;- To create a foreign key between table, click the middle mouse button on the table columns you want to connect.&lt;/p&gt;&lt;p&gt;- &lt;span style=&quot; font-weight:700;&quot;&gt;All diagram changes remain pending until you explicitly commit or roll them back&lt;/span&gt; using the toolbar buttons in the top-left corner of the diagram.&lt;/p&gt;&lt;p&gt;&lt;a href=&quot;https://github.com/pawelsalawa/sqlitestudio/wiki/ERD-plugin-manual#usage&quot;&gt;&lt;span style=&quot; font-weight:700; text-decoration: underline; color:#058800;&quot;&gt;Learn more (click to open online help page)&lt;/span&gt;&lt;/a&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;- 按住空格键并拖动鼠标，即可自由平移图表，无需选择任何项目。&lt;/p&gt;&lt;p&gt;- 使用鼠标滚轮放大和缩小。&lt;/p&gt;&lt;p&gt;- 取消选择表格（或单击侧边栏工具栏中的“提交”），即可将侧边栏的更改应用到图表。&lt;/p&gt;&lt;p&gt;- 按 Esc 键（或单击侧边栏工具栏中的“回滚”），即可放弃侧边栏的更改。&lt;/p&gt;&lt;p&gt;- 双击表名或列名，即可直接编辑名称。&lt;/p&gt;&lt;p&gt;- 按住 Shift 键并双击列名，即可编辑列详细信息（数据类型、约束）。&lt;/p&gt;&lt;p&gt;- 要在表之间创建外键，单击要连接的表格列上的鼠标中键。所有图表更改将保持待处理状态，直到您使用图表左上角的工具栏按钮显式提交或回滚它们。了解更多信息（点击打开在线帮助页面）&lt;/p&gt;&lt;p&gt;&lt;a href=&quot;https://github.com/pawelsalawa/sqlitestudio/wiki/ERD-plugin-manual#usage&quot;&gt;&lt;span style=&quot; font-weight:700; text-decoration: underline; color:#058800;&quot;&gt;Learn more (click to open online help page)&lt;/span&gt;&lt;/a&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="180"/>
        <source>The number of changes pending for commit. Click to see details.</source>
        <comment>ERD editor</comment>
        <translation>待提交的更改数量。点击查看详情。</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="189"/>
        <source>Add a foreign key</source>
        <comment>ERD editor</comment>
        <translation>添加外键</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="191"/>
        <source>Delete selected items</source>
        <comment>ERD editor</comment>
        <translation>删除所选项目</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="197"/>
        <source>Auto-arrange (local forces)</source>
        <comment>ERD editor</comment>
        <translation>自动排列（局部力）</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="198"/>
        <source>Auto-arrange (global balance)</source>
        <comment>ERD editor</comment>
        <translation>自动排列（全局平衡）</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="212"/>
        <source>Set table color</source>
        <comment>ERD editor</comment>
        <translation>设置表颜色</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="240"/>
        <source>Use straight line</source>
        <comment>ERD editor</comment>
        <translation>使用直线</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="241"/>
        <source>Use curvy line</source>
        <comment>ERD editor</comment>
        <translation>使用曲线线</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="242"/>
        <source>Use square line</source>
        <comment>ERD editor</comment>
        <translation>使用方形线</translation>
    </message>
    <message>
        <location filename="../erdwindow.cpp" line="253"/>
        <source>Choose line type</source>
        <comment>ERD editor</comment>
        <translation>选择线类型</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../changes/erdchangedeleteentity.cpp" line="48"/>
        <source>Drop table &quot;%1&quot;.</source>
        <comment>ERD editor</comment>
        <translation>拖放表 &quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../changes/erdchangemodifyentity.cpp" line="61"/>
        <source>Modify table &quot;%1&quot;.</source>
        <comment>ERD editor</comment>
        <translation>修改表 &quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../changes/erdchangenewentity.cpp" line="54"/>
        <source>Create table &quot;%1&quot;.</source>
        <comment>ERD editor</comment>
        <translation>创建表 &quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../changes/erdeffectivechangemerger.cpp" line="31"/>
        <source>Failed to create in-memory databases required for change compacting.</source>
        <translation>创建变更压缩所需的内存数据库失败。</translation>
    </message>
    <message>
        <location filename="../changes/erdeffectivechangemerger.cpp" line="399"/>
        <source>Drop tables: %1</source>
        <comment>ERD editor</comment>
        <translation>拖放表： %1</translation>
    </message>
    <message>
        <location filename="../scene/erdconnection.cpp" line="149"/>
        <source>Could not commit changes for finalized ERD connection.</source>
        <translation>无法提交已完成的 ERD 连接的更改。</translation>
    </message>
    <message>
        <location filename="../scene/erdconnection.cpp" line="155"/>
        <source>Update relationship from &quot;%1&quot;-&quot;%2&quot; to &quot;%1&quot;-&quot;%3&quot;.</source>
        <translation>将关系从 &quot;%1&quot;-&quot;%2&quot; 更新为 &quot;%1&quot;-&quot;%3&quot;。</translation>
    </message>
    <message>
        <location filename="../scene/erdconnection.cpp" line="157"/>
        <source>Create relationship between &quot;%1&quot; and &quot;%2&quot;.</source>
        <translation>创建 &quot;%1&quot; 和 &quot;%2&quot; 之间的关系。</translation>
    </message>
    <message>
        <location filename="../changes/erdchangemoveentity.cpp" line="28"/>
        <source>Move table &quot;%1&quot;</source>
        <translation>移动表 &quot;%1&quot;</translation>
    </message>
</context>
</TS>
