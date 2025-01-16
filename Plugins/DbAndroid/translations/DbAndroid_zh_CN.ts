<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh-CN" sourcelanguage="en">
  <context>
    <name>DbAndroid</name>
    <message>
      <location filename="../dbandroid.cpp" line="40"/>
      <source>Invalid or incomplete Android Database URL.</source>
      <translation>无效或不完整的 Android 数据库 URL。</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="55"/>
      <source>Android database URL</source>
      <translation>Android 数据库 URL</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="56"/>
      <source>Select Android database</source>
      <translation>选择 Android 数据库</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="151"/>
      <source>Select ADB</source>
      <translation>选择 ADB</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="173"/>
      <source>Using Android Debug Bridge: %1</source>
      <translation>使用 Android 调试桥（ADB）：%1</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="183"/>
      <source>You can grab Android connector JAR file from Tools menu. It&apos;s required for 2 of 3 connections supported by the Android plugin. For more details read plugin&apos;s documentation on &lt;a href=&quot;%1&quot;&gt;SQLiteStudio&apos;s wiki page.&lt;/a&gt;</source>
      <translation>您可以从工具菜单取得 Android 连接器 JAR 文件。 It&apos;s required for 2 of 3 connections supported by the Android plugin. 更多细节见 SQLiteStudio wiki 页面上的&lt;a href=&quot;%1&quot;&gt;插件文档&lt;/a&gt;。</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="198"/>
      <source>Could not find Android Debug Bridge application. &lt;a href=&quot;%1&quot;&gt;Click here&lt;/a&gt; to point out the location of the ADB application, otherwise the %2 plugin will not support USB cable connections, only the network connection.</source>
      <translation>未能找到 Android 调试桥（ADB）应用程序。&lt;a href=&quot;%1&quot;&gt;单击此处&lt;/a&gt;以指明 ADB 应用程序的位置，否则 %2 插件将无法支持 USB 线缆连接，只通过网络连接。</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="217"/>
      <source>The selected ADB is incorrect.</source>
      <translation>所选的 ADB 不正确。</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="218"/>
      <source>Would you like to select another one, or leave it unconfigured?</source>
      <translation>您是否要选择另一个，还是保持其未配置状态？</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="242"/>
      <source>Save JAR file</source>
      <translation>保存 JAR 文件</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="216"/>
      <source>Invalid ADB</source>
      <translation>无效的 ADB</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="219"/>
      <source>Select another ADB</source>
      <translation>选择其他 ADB</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="220"/>
      <source>Leave unconfigured</source>
      <translation>不做配置</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="192"/>
      <source>Get Android connector JAR file</source>
      <translation>获取 Android 连接器 JAR 文件</translation>
    </message>
  </context>
  <context>
    <name>DbAndroidInstance</name>
    <message>
      <location filename="../dbandroidinstance.cpp" line="113"/>
      <source>Android SQLite driver does not support loadable extensions.</source>
      <translation>Android SQLite 驱动程序不支持可加载的扩展。</translation>
    </message>
    <message>
      <location filename="../dbandroidinstance.cpp" line="217"/>
      <source>Connection with Android database &apos;%1&apos; lost.</source>
      <translation>与 Android 数据库 &apos;%1&apos; 的连接丢失。</translation>
    </message>
  </context>
  <context>
    <name>DbAndroidJsonConnection</name>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="175"/>
      <source>Cannot connect to device %1, because it&apos;s not visible from your computer.</source>
      <translation>由于&apos;在您的机器上不可见，无法连接到设备%1</translation>
    </message>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="182"/>
      <source>Failed to create port forwarding for device %1 for port %2.</source>
      <translation>为设备 %1 创建 %2 端口转发失败。</translation>
    </message>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="196"/>
      <source>Could not connect to network host: %1:%2</source>
      <translation>无法连接到网络主机：%1:%2</translation>
    </message>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="210"/>
      <source>Cannot connect to %1:%2, because password is invalid.</source>
      <translation>无法连接到 %1:%2，密码无效。</translation>
    </message>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="301"/>
      <source>Unable to execute query on Android device (connection was closed): %1</source>
      <translation>无法在 Android 设备上执行查询（连接已关闭）：%1</translation>
    </message>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="313"/>
      <source>Error while parsing response from Android: %1</source>
      <translation>解析来自 Android 响应出错：%1</translation>
    </message>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="321"/>
      <source>Generic error from Android: %1</source>
      <translation>来自 Android 的一般错误：%1</translation>
    </message>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="335"/>
      <location filename="../dbandroidjsonconnection.cpp" line="342"/>
      <source>Missing &apos;columns&apos; in response from Android.</source>
      <translation>来自 Android 的响应中缺少 &apos;columns&apos;。</translation>
    </message>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="363"/>
      <source>Response from Android has missing data for column &apos;%1&apos; in row %2.</source>
      <translation>来自 Android 的响应中，缺少数据行 %2、列 &apos;%1&apos; 的数据。</translation>
    </message>
  </context>
  <context>
    <name>DbAndroidPathDialog</name>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="20"/>
      <source>Android database URL</source>
      <translation>Android 数据库 URL</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="26"/>
      <source>Connection method</source>
      <translation>连接方式</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="32"/>
      <source>USB cable - port forwarding</source>
      <translation>USB 线缆 - 端口转发</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="42"/>
      <source>USB cable - sqlite3 command</source>
      <translation>USB 线缆 - sqlite3 命令</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="49"/>
      <source>Network (IP address)</source>
      <translation>网络（IP 地址）</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="59"/>
      <source>Device</source>
      <translation>设备</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="71"/>
      <source>IP address</source>
      <translation>IP 地址</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="93"/>
      <source>Port</source>
      <translation>端口</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="115"/>
      <source>Remote access password</source>
      <translation>远程访问密码</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="127"/>
      <source>&lt;p&gt;This is password configured in the SQLiteStudio service being embeded in the Android application.&lt;/p&gt;</source>
      <translation>&lt;p&gt;这是在 Android 应用程序中嵌入的为 SQLiteStudio 服务配置的密码。&lt;/p&gt;</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="140"/>
      <source>Application</source>
      <translation>应用程序</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="155"/>
      <source>Filter</source>
      <translation>筛选器</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="168"/>
      <source>Database</source>
      <translation>数据库</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="177"/>
      <source>Create a new database directly on the device.</source>
      <translation>直接在该设备上新建一个数据库。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="187"/>
      <source>Delete currently selected database from the device. The currently selected database is the one picked in the list on the left of this button.</source>
      <translation>从设备中删除当前选中的数据库。当前选中的数据库是此按钮左侧的列表中所选择的数据库。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="362"/>
      <source>Enter valid IP address.</source>
      <translation>请键入有效的 IP 地址。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="367"/>
      <source>Pick Android device.</source>
      <translation>选择 Android 设备。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="371"/>
      <source>Pick Android database.</source>
      <translation>选择 Android 数据库。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="395"/>
      <source>Selected Android application is unknown, or not debuggable.</source>
      <translation>所选的 Android 应用程序未知或者非可调试。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="422"/>
      <source>Create new database</source>
      <translation>创建新数据库</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="422"/>
      <source>Please provide name for the new database.
It&apos;s the name which Android application will use to connect to the database:</source>
      <translation>请提供新数据库的名称。
这将是 Android 应用程序用来连接该数据库的名称：</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="430"/>
      <location filename="../dbandroidpathdialog.cpp" line="439"/>
      <location filename="../dbandroidpathdialog.cpp" line="448"/>
      <source>Invalid name</source>
      <translation>无效名称</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="430"/>
      <source>Database with the same name (%1) already exists on the device.
The name must be unique.</source>
      <translation>设备上已存在相同名称的数据库（%1）。
名称不能重复。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="439"/>
      <source>Could not create database &apos;%1&apos;, because could not connect to the device.</source>
      <translation>无法创建数据库 &apos;%1&apos;，无法连接到该设备。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="448"/>
      <source>Could not create database &apos;%1&apos;.
Details: %2</source>
      <translation>无法创建数据库 &apos;%1&apos;。
详情：%2</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="463"/>
      <source>Delete database</source>
      <translation>删除数据库</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="463"/>
      <source>Are you sure you want to delete database &apos;%1&apos; from %2?</source>
      <translation>确定从 %2 中删除数据库 &apos;%1&apos; 吗？</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="484"/>
      <location filename="../dbandroidpathdialog.cpp" line="490"/>
      <source>Error deleting</source>
      <translation>删除出错</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="484"/>
      <source>Could not connect to %1 in order to delete database &apos;%2&apos;.</source>
      <translation>无法连接到 %1 以删除数据库 &apos;%2&apos;。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="490"/>
      <source>Could not delete database named &apos;%1&apos; from the device.
Android device refused deletion, or it was impossible.</source>
      <translation>无法从该设备中删除数据库 &apos;%1&apos;。
Android 设备拒绝或无法完成删除。</translation>
    </message>
  </context>
  <context>
    <name>DbAndroidShellConnection</name>
    <message>
      <location filename="../dbandroidshellconnection.cpp" line="31"/>
      <source>Cannot connect to device %1, because it&apos;s not visible from your computer.</source>
      <translation>由于&apos;在您的机器上不可见，无法连接到设备%1</translation>
    </message>
    <message>
      <location filename="../dbandroidshellconnection.cpp" line="46"/>
      <source>Cannot connect to device %1, because the application %2 doesn&apos;t seem to be installed on the device.</source>
      <translation>无法连接设备 %1，应用程序 %2 似乎没有在该设备上安装。</translation>
    </message>
    <message>
      <location filename="../dbandroidshellconnection.cpp" line="56"/>
      <source>Cannot connect to device %1, because the application %2 is not debuggable.</source>
      <translation>无法连接设备 %1，应用程序 %2 不可调试。</translation>
    </message>
    <message>
      <location filename="../dbandroidshellconnection.cpp" line="65"/>
      <source>Cannot connect to device %1, because &apos;%2&apos; command doesn&apos;t seem to be available on the device.</source>
      <translation>无法连接设备 %1，命令 &apos;%2&apos; 在该设备上似乎不可用。</translation>
    </message>
    <message>
      <location filename="../dbandroidshellconnection.cpp" line="77"/>
      <source>Cannot connect to device %1, because &apos;%2&apos; database cannot be accessed on the device.</source>
      <translation>无法连接设备 %1，因无法访问该设备上的 &apos;%2&apos; 数据库。</translation>
    </message>
    <message>
      <location filename="../dbandroidshellconnection.cpp" line="90"/>
      <source>Cannot connect to device %1, because &apos;%2&apos; database cannot be accessed on the device. Details: %3</source>
      <translation>无法连接设备 %1，因无法访问该设备上的 &apos;%2&apos; 数据库。详细信息：%3</translation>
    </message>
    <message>
      <location filename="../dbandroidshellconnection.cpp" line="126"/>
      <source>Cannot get list of databases for application %1. Details: %2</source>
      <translation>无法获取应用程序 %1 的数据库列表。详细信息：%2</translation>
    </message>
    <message>
      <location filename="../dbandroidshellconnection.cpp" line="207"/>
      <location filename="../dbandroidshellconnection.cpp" line="214"/>
      <source>Could not execute query on database &apos;%1&apos;: %2</source>
      <translation>无法在数据库 &apos;%1&apos; 上执行查询：%2</translation>
    </message>
  </context>
  <context>
    <name>QObject</name>
    <message>
      <location filename="../sqlqueryandroid.cpp" line="101"/>
      <source>Cannot bind argument &apos;%1&apos; of the query, because it&apos;s value is missing.</source>
      <translation>无法在此查询中绑定参数 &apos;%1&apos;，因为缺少它的值。</translation>
    </message>
  </context>
</TS>
