#ifndef DIALOGSIZEHANDLER_H
#define DIALOGSIZEHANDLER_H

#include <QObject>
#include <QRect>

class QTimer;

class DialogSizeHandler : public QObject
{
    Q_OBJECT
public:
    explicit DialogSizeHandler(QObject *parent);
    DialogSizeHandler(const QString& key, QObject *parent);
    virtual ~DialogSizeHandler();

    static void applyFor(QObject *parent);
    static void applyFor(const QString& key, QObject *parent);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    static const constexpr char* CONFIG_GROUP = "DialogDimensions";

    QString configKey;
    QTimer* saveTimer = nullptr;
    QRect recentGeometry;

public slots:
    void doSave();
};

#endif // DIALOGSIZEHANDLER_H
