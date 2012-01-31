#ifndef UIAPPLICATION_H
#define UIAPPLICATION_H

#include "ui_mainwindow.h"
#include "IUIBuilder.h"
#include "UserPreferences.h"
#include <dukexcore/dkxNodeManager.h>
#include <dukexcore/dkxSession.h>
//#include <QDeclarativeEngine>
#include <QtGui>

class UIRenderWindow;
class UIPluginDialog;
//QT_BEGIN_NAMESPACE
//class QDeclarativeItem;
//QT_END_NAMESPACE

class UIApplication : public QMainWindow, public IUIBuilder {

Q_OBJECT

public:
    UIApplication(const std::string&, const short&);
    ~UIApplication() {
    }

public:
    // IUIBuilder Interface
//    QMenu * createMenu(QObject* _plugin, const QString & _title);
//    QWidget* createWindow(QObject* _plugin, const Qt::DockWidgetArea & _area, const QString & _title);
    bool createWindow(QObject* _plugin, UIWidget* _widget, const Qt::DockWidgetArea & _area, const QString & _title);
//    QDeclarativeItem* createQMLWindow(QObject* _plugin, const QUrl &qmlfile, const Qt::DockWidgetArea & _area, const QString & _title);

private slots:
    void closeEvent(QCloseEvent *event);
    void timerEvent(QTimerEvent *event);
    void keyPressEvent(QKeyEvent * event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void resizeCentralWidget(const QSize& resolution);
    void updateRecentFilesMenu();

private slots:
    // file
    void open(const QString &);
    void open();
    void openRecent();
    // control
    void playStop();
    void previousFrame();
    void nextFrame();
    void firstFrame();
    void lastFrame();
    void previousShot();
    void nextShot();
    // view
    void fullscreen();
    void toggleFitMode();
    void fitToNormalSize();
    void fitImageToWindowWidth();
    void fitImageToWindowHeight();
    void stretchImageToWindow();
    void zoom(double);
    void pan(double, double);
    // ?
    void about();
    void aboutPlugins();

private:
    Ui::mainWindow ui;
//    QDeclarativeEngine m_Engine;
    NodeManager m_Manager;
    Session::ptr m_Session;
    UserPreferences m_Preferences;
    UIRenderWindow* m_RenderWindow;
    UIPluginDialog* m_PluginDialog;
    QLabel* m_statusInfo;
    int m_timerID;
};

#endif // UIAPPLICATION_H