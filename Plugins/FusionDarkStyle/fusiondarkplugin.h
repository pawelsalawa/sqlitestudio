#ifndef STYLEPLUGIN_H
#define STYLEPLUGIN_H

#include <QStylePlugin>

class FusionDarkPlugin : public QStylePlugin
{
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface" FILE "FusionDarkStyle.json")

    public:
        explicit FusionDarkPlugin(QObject *parent = nullptr);

    private:
        QStyle *create(const QString &key) override;
};

#endif // STYLEPLUGIN_H
