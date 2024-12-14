#-------------------------------------------------
#
# Project created by QtCreator 2013-02-28T23:22:10
#
#-------------------------------------------------

QT       += core gui uitools widgets xml svg

include($$PWD/../common.pri)
include($$PWD/../utils.pri)

OBJECTS_DIR = $$OBJECTS_DIR/guiSQLiteStudio
MOC_DIR = $$MOC_DIR/guiSQLiteStudio
UI_DIR = $$UI_DIR/guiSQLiteStudio

linux: {
    portable: {
        DESTDIR = $$DESTDIR/lib
    }
}

portable: {
    DEFINES += PORTABLE_CONFIG
}

TARGET = guiSQLiteStudio
TEMPLATE = lib

CONFIG  += c++20
QMAKE_CXXFLAGS += -pedantic

DEFINES += GUISQLITESTUDIO_LIBRARY

SOURCES +=\
    common/dbcombobox.cpp \
    common/dialogsizehandler.cpp \
    common/immediatetooltip.cpp \
    common/mouseshortcut.cpp \
    constraints/columngeneratedpanel.cpp \
    datagrid/fkcombobox.cpp \
    datagrid/sqlqueryitemlineedit.cpp \
    extendedpalette.cpp \
        mainwindow.cpp \
    iconmanager.cpp \
    dbtree/dbtreemodel.cpp \
    dbtree/dbtreeitem.cpp \
    dbtree/dbtree.cpp \
    dbtree/dbtreeview.cpp \
    actionentry.cpp \
    multieditor/multieditorfk.cpp \
    style.cpp \
    uiutils.cpp \
    dbtree/dbtreeitemdelegate.cpp \
    dbtree/dbtreeitemfactory.cpp \
    sqleditor.cpp \
    datagrid/sqlquerymodel.cpp \
    datagrid/sqldatasourcequerymodel.cpp \
    dblistmodel.cpp \
    mdiarea.cpp \
    statusfield.cpp \
    common/tablewidget.cpp \
    datagrid/sqlqueryitem.cpp \
    datagrid/sqlqueryview.cpp \
    datagrid/sqlquerymodelcolumn.cpp \
    datagrid/sqlqueryitemdelegate.cpp \
    common/extlineedit.cpp \
    common/intvalidator.cpp \
    common/widgetcover.cpp \
    mdiwindow.cpp \
    mdichild.cpp \
    taskbar.cpp \
    multieditor/multieditor.cpp \
    multieditor/multieditorwidget.cpp \
    widgetresizer.cpp \
    multieditor/multieditortext.cpp \
    multieditor/multieditornumeric.cpp \
    common/numericspinbox.cpp \
    multieditor/multieditordatetime.cpp \
    multieditor/multieditordate.cpp \
    multieditor/multieditortime.cpp \
    formview.cpp \
    multieditor/multieditorbool.cpp \
    multieditor/multieditorhex.cpp \
    qhexedit2/xbytearray.cpp \
    qhexedit2/qhexedit_p.cpp \
    qhexedit2/qhexedit.cpp \
    qhexedit2/commands.cpp \
    multieditor/multieditordialog.cpp \
    completer/completerwindow.cpp \
    completer/completermodel.cpp \
    completer/completeritemdelegate.cpp \
    completer/completerview.cpp \
    dialogs/searchtextdialog.cpp \
    searchtextlocator.cpp \
    windows/codesnippeteditor.cpp \
    windows/codesnippeteditormodel.cpp \
    windows/tablewindow.cpp \
    windows/editorwindow.cpp \
    datagrid/sqltablemodel.cpp \
    dataview.cpp \
    windows/tablestructuremodel.cpp \
    windows/tableconstraintsmodel.cpp \
    dialogs/columndialog.cpp \
    dialogs/columndialogconstraintsmodel.cpp \
    common/extactioncontainer.cpp \
    constraints/tableprimarykeypanel.cpp \
    constraints/constraintpanel.cpp \
    constraints/tableforeignkeypanel.cpp \
    constraints/tableuniquepanel.cpp \
    constraints/tablepkanduniquepanel.cpp \
    constraints/tablecheckpanel.cpp \
    constraints/columncheckpanel.cpp \
    constraints/constraintcheckpanel.cpp \
    constraints/columnforeignkeypanel.cpp \
    constraints/columnprimarykeypanel.cpp \
    constraints/columnuniquepanel.cpp \
    constraints/columnuniqueandnotnullpanel.cpp \
    constraints/columnnotnullpanel.cpp \
    constraints/columncollatepanel.cpp \
    constraints/columndefaultpanel.cpp \
    dialogs/constraintdialog.cpp \
    dialogs/newconstraintdialog.cpp \
    windows/constrainttabmodel.cpp \
    dialogs/messagelistdialog.cpp \
    windows/viewwindow.cpp \
    dialogs/configdialog.cpp \
    uiconfig.cpp \
    dialogs/indexdialog.cpp \
    sqlview.cpp \
    dialogs/triggerdialog.cpp \
    dialogs/triggercolumnsdialog.cpp \
    dbobjectdialogs.cpp \
    common/fontedit.cpp \
    configwidgets/styleconfigwidget.cpp \
    common/colorbutton.cpp \
    formmanager.cpp \
    configwidgets/combodatawidget.cpp \
    dialogs/ddlpreviewdialog.cpp \
    windows/ddlhistorywindow.cpp \
    common/userinputfilter.cpp \
    datagrid/sqlqueryrownummodel.cpp \
    windows/functionseditor.cpp \
    windows/functionseditormodel.cpp \
    sqlitesyntaxhighlighter.cpp \
    windows/collationseditor.cpp \
    selectabledbmodel.cpp \
    windows/collationseditormodel.cpp \
    qtscriptsyntaxhighlighter.cpp \
    icon.cpp \
    configmapper.cpp \
    dialogs/exportdialog.cpp \
    dbobjlistmodel.cpp \
    common/verifiablewizardpage.cpp \
    selectabledbobjmodel.cpp \
    common/widgetstateindicator.cpp \
    configwidgets/listtostringlisthash.cpp \
    dialogs/versionconvertsummarydialog.cpp \
    sqlcompareview.cpp \
    dialogs/errorsconfirmdialog.cpp \
    dialogs/sortdialog.cpp \
    dialogs/importdialog.cpp \
    dialogs/populatedialog.cpp \
    dialogs/populateconfigdialog.cpp \
    common/configradiobutton.cpp \
    uiloader.cpp \
    common/fileedit.cpp \
    uiscriptingcombo.cpp \
    uiscriptingedit.cpp \
    uicustomicon.cpp \
    uiurlbutton.cpp \
    common/configcombobox.cpp \
    dialogs/dbdialog.cpp \
    uidebug.cpp \
    debugconsole.cpp \
    common/extactionprototype.cpp \
    dialogs/aboutdialog.cpp \
    dialogs/newversiondialog.cpp \
    dialogs/quitconfirmdialog.cpp \
    common/datawidgetmapper.cpp \
    dialogs/languagedialog.cpp \
    common/ipvalidator.cpp \
    dialogs/cssdebugdialog.cpp \
    themetuner.cpp \
    dialogs/indexexprcolumndialog.cpp \
    common/centerediconitemdelegate.cpp \
    datagrid/sqlviewmodel.cpp \
    common/exttableview.cpp \
    common/exttablewidget.cpp \
    windows/sqliteextensioneditor.cpp \
    windows/sqliteextensioneditormodel.cpp \
    dialogs/bindparamsdialog.cpp \
    dialogs/execfromfiledialog.cpp \
    dialogs/fileexecerrorsdialog.cpp

HEADERS  += mainwindow.h \
    common/dbcombobox.h \
    common/dialogsizehandler.h \
    common/immediatetooltip.h \
    common/mouseshortcut.h \
    constraints/columngeneratedpanel.h \
    datagrid/fkcombobox.h \
    datagrid/sqlqueryitemlineedit.h \
    extendedpalette.h \
    iconmanager.h \
    dbtree/dbtreemodel.h \
    dbtree/dbtreeitem.h \
    dbtree/dbtree.h \
    dbtree/dbtreeview.h \
    actionentry.h \
    multieditor/multieditorfk.h \
    style.h \
    uiutils.h \
    dbtree/dbtreeitemdelegate.h \
    dbtree/dbtreeitemfactory.h \
    sqleditor.h \
    datagrid/sqlquerymodel.h \
    datagrid/sqldatasourcequerymodel.h \
    dblistmodel.h \
    mdiarea.h \
    statusfield.h \
    common/tablewidget.h \
    datagrid/sqlqueryitem.h \
    datagrid/sqlqueryview.h \
    datagrid/sqlquerymodelcolumn.h \
    datagrid/sqlqueryitemdelegate.h \
    common/extlineedit.h \
    common/intvalidator.h \
    common/widgetcover.h \
    mdiwindow.h \
    mdichild.h \
    taskbar.h \
    multieditor/multieditor.h \
    multieditor/multieditorwidgetplugin.h \
    multieditor/multieditorwidget.h \
    widgetresizer.h \
    multieditor/multieditortext.h \
    multieditor/multieditornumeric.h \
    common/numericspinbox.h \
    multieditor/multieditordatetime.h \
    multieditor/multieditordate.h \
    multieditor/multieditortime.h \
    formview.h \
    multieditor/multieditorbool.h \
    multieditor/multieditorhex.h \
    qhexedit2/xbytearray.h \
    qhexedit2/qhexedit_p.h \
    qhexedit2/qhexedit.h \
    qhexedit2/commands.h \
    multieditor/multieditordialog.h \
    completer/completerwindow.h \
    completer/completermodel.h \
    completer/completeritemdelegate.h \
    completer/completerview.h \
    dialogs/searchtextdialog.h \
    searchtextlocator.h \
    windows/codesnippeteditor.h \
    windows/codesnippeteditormodel.h \
    windows/tablewindow.h \
    windows/editorwindow.h \
    datagrid/sqltablemodel.h \
    dataview.h \
    windows/tablestructuremodel.h \
    windows/tableconstraintsmodel.h \
    dialogs/columndialog.h \
    dialogs/columndialogconstraintsmodel.h \
    common/extactioncontainer.h \
    constraints/tableprimarykeypanel.h \
    constraints/constraintpanel.h \
    constraints/tableforeignkeypanel.h \
    constraints/tableuniquepanel.h \
    constraints/tablepkanduniquepanel.h \
    constraints/tablecheckpanel.h \
    constraints/columncheckpanel.h \
    constraints/constraintcheckpanel.h \
    constraints/columnforeignkeypanel.h \
    constraints/columnprimarykeypanel.h \
    constraints/columnuniquepanel.h \
    constraints/columnuniqueandnotnullpanel.h \
    constraints/columnnotnullpanel.h \
    constraints/columncollatepanel.h \
    constraints/columndefaultpanel.h \
    dialogs/constraintdialog.h \
    dialogs/newconstraintdialog.h \
    windows/constrainttabmodel.h \
    dialogs/messagelistdialog.h \
    windows/viewwindow.h \
    uiconfig.h \
    dialogs/indexdialog.h \
    sqlview.h \
    dialogs/triggerdialog.h \
    dialogs/triggercolumnsdialog.h \
    dbobjectdialogs.h \
    common/fontedit.h \
    customconfigwidgetplugin.h \
    configwidgets/styleconfigwidget.h \
    common/colorbutton.h \
    formmanager.h \
    configwidgets/combodatawidget.h \
    dialogs/ddlpreviewdialog.h \
    windows/ddlhistorywindow.h \
    common/userinputfilter.h \
    datagrid/sqlqueryrownummodel.h \
    windows/functionseditor.h \
    windows/functionseditormodel.h \
    syntaxhighlighterplugin.h \
    sqlitesyntaxhighlighter.h \
    windows/collationseditor.h \
    selectabledbmodel.h \
    windows/collationseditormodel.h \
    qtscriptsyntaxhighlighter.h \
    icon.h \
    configmapper.h \
    dialogs/exportdialog.h \
    dbobjlistmodel.h \
    common/verifiablewizardpage.h \
    selectabledbobjmodel.h \
    common/widgetstateindicator.h \
    configwidgets/listtostringlisthash.h \
    dialogs/versionconvertsummarydialog.h \
    sqlcompareview.h \
    dialogs/errorsconfirmdialog.h \
    dialogs/sortdialog.h \
    dialogs/importdialog.h \
    dialogs/populatedialog.h \
    dialogs/populateconfigdialog.h \
    common/configradiobutton.h \
    uiloader.h \
    common/fileedit.h \
    uiscriptingcombo.h \
    uiloaderpropertyhandler.h \
    uiscriptingedit.h \
    uicustomicon.h \
    uiurlbutton.h \
    common/configcombobox.h \
    dialogs/configdialog.h \
    dialogs/dbdialog.h \
    uidebug.h \
    debugconsole.h \
    common/extactionprototype.h \
    dialogs/aboutdialog.h \
    dialogs/newversiondialog.h \
    guiSQLiteStudio_global.h \
    dialogs/quitconfirmdialog.h \
    common/datawidgetmapper.h \
    dialogs/languagedialog.h \
    common/ipvalidator.h \
    dialogs/cssdebugdialog.h \
    themetuner.h \
    dialogs/indexexprcolumndialog.h \
    common/centerediconitemdelegate.h \
    datagrid/sqlviewmodel.h \
    common/exttableview.h \
    common/exttablewidget.h \
    windows/sqliteextensioneditor.h \
    windows/sqliteextensioneditormodel.h \
    dialogs/bindparamsdialog.h \
    common/bindparam.h \
    dialogs/execfromfiledialog.h \
    dialogs/fileexecerrorsdialog.h

FORMS    += mainwindow.ui \
    constraints/columngeneratedpanel.ui \
    dbtree/dbtree.ui \
    statusfield.ui \
    completer/completerwindow.ui \
    dialogs/searchtextdialog.ui \
    windows/codesnippeteditor.ui \
    windows/tablewindow.ui \
    windows/editorwindow.ui \
    dialogs/columndialog.ui \
    constraints/tableforeignkeypanel.ui \
    constraints/tablepkanduniquepanel.ui \
    constraints/constraintcheckpanel.ui \
    constraints/columnforeignkeypanel.ui \
    constraints/columnprimarykeypanel.ui \
    constraints/columnuniqueandnotnullpanel.ui \
    constraints/columncollatepanel.ui \
    constraints/columndefaultpanel.ui \
    dialogs/constraintdialog.ui \
    dialogs/newconstraintdialog.ui \
    dialogs/messagelistdialog.ui \
    windows/viewwindow.ui \
    dialogs/configdialog.ui \
    dialogs/indexdialog.ui \
    dialogs/triggerdialog.ui \
    dialogs/triggercolumnsdialog.ui \
    common/fontedit.ui \
    forms/sqlformatterplugin.ui \
    dialogs/ddlpreviewdialog.ui \
    windows/ddlhistorywindow.ui \
    windows/functionseditor.ui \
    windows/collationseditor.ui \
    dialogs/exportdialog.ui \
    dialogs/versionconvertsummarydialog.ui \
    dialogs/errorsconfirmdialog.ui \
    dialogs/sortdialog.ui \
    dialogs/importdialog.ui \
    dialogs/populatedialog.ui \
    dialogs/populateconfigdialog.ui \
    dialogs/dbdialog.ui \
    debugconsole.ui \
    dialogs/aboutdialog.ui \
    dialogs/newversiondialog.ui \
    dialogs/quitconfirmdialog.ui \
    dialogs/languagedialog.ui \
    dialogs/cssdebugdialog.ui \
    dialogs/indexexprcolumndialog.ui \
    windows/sqliteextensioneditor.ui \
    dialogs/bindparamsdialog.ui \
    dialogs/execfromfiledialog.ui \
    dialogs/fileexecerrorsdialog.ui

RESOURCES += \
    icons.qrc \
    guiSQLiteStudio.qrc

OTHER_FILES +=

unix: {
    target.path = $$LIBDIR
    INSTALLS += target
}

LIBS += -lcoreSQLiteStudio

DISTFILES += \
    general.css


























