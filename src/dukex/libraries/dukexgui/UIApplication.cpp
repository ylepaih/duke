#include "UIApplication.h"
//#include "UIView.h"
#include "UIRenderWindow.h"
#include "UIPluginDialog.h"
#include <dukexcore/nodes/Commons.h>
#include <boost/filesystem.hpp>
//#include <QDeclarativeComponent>
//#include <QDeclarativeContext>
//#include <QDeclarativeItem>
#include <iostream>

#ifndef BUILD_INFORMATION
#define BUILD_INFORMATION ""
#endif

UIApplication::UIApplication(const std::string& _path, const short& _port) :
    m_Session(new Session()),//
                    m_RenderWindow(new UIRenderWindow(this)), //
                    m_PluginDialog(new UIPluginDialog(this, this, &m_Manager, qApp->applicationDirPath())), //
                    m_timerID(0) {
    ui.setupUi(this);

    setCentralWidget(m_RenderWindow);
    // status bar
    m_statusInfo = new QLabel();
    m_statusInfo->setText("Starting...");
    menuBar()->setCornerWidget(m_statusInfo);
    // preferences
    m_Preferences.loadShortcuts(this);
    m_Preferences.loadFileHistory();
    updateRecentFilesMenu();
    // File Actions
    connect(ui.openAction, SIGNAL(triggered()), this, SLOT(open()));
    connect(ui.quitAction, SIGNAL(triggered()), this, SLOT(close()));
    // Control Actions
    connect(ui.playStopAction, SIGNAL(triggered()), this, SLOT(playStop()));
    connect(ui.nextFrameAction, SIGNAL(triggered()), this, SLOT(nextFrame()));
    connect(ui.previousFrameAction, SIGNAL(triggered()), this, SLOT(previousFrame()));
    connect(ui.firstFrameAction, SIGNAL(triggered()), this, SLOT(firstFrame()));
    connect(ui.lastFrameAction, SIGNAL(triggered()), this, SLOT(lastFrame()));
    connect(ui.nextShotAction, SIGNAL(triggered()), this, SLOT(nextShot()));
    connect(ui.previousShotAction, SIGNAL(triggered()), this, SLOT(previousShot()));
    // View Actions
    connect(ui.fullscreenAction, SIGNAL(triggered()), this, SLOT(fullscreen()));
    connect(ui.toggleFitModeAction, SIGNAL(triggered()), this, SLOT(toggleFitMode()));
    connect(ui.fitImageTo11Action, SIGNAL(triggered()), this, SLOT(fitToNormalSize()));
    connect(ui.fitImageToWindowWidthAction, SIGNAL(triggered()), this, SLOT(fitImageToWindowWidth()));
    connect(ui.fitImageToWindowHeightAction, SIGNAL(triggered()), this, SLOT(fitImageToWindowHeight()));
    connect(ui.stretchImageToWindowAction, SIGNAL(triggered()), this, SLOT(stretchImageToWindow()));
    connect(m_RenderWindow, SIGNAL(zoomChanged(double)), this, SLOT(zoom(double)));
    connect(m_RenderWindow, SIGNAL(panChanged(double,double)), this, SLOT(pan(double, double)));
    // About Actions
    connect(ui.aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui.aboutPluginsAction, SIGNAL(triggered()), this, SLOT(aboutPlugins()));

    PlaylistNode::ptr p = PlaylistNode::ptr(new PlaylistNode());
    m_Manager.addNode(p, m_Session);
    TransportNode::ptr t = TransportNode::ptr(new TransportNode());
    m_Manager.addNode(t, m_Session);
    FitNode::ptr f = FitNode::ptr(new FitNode());
    m_Manager.addNode(f, m_Session);
    GradingNode::ptr g = GradingNode::ptr(new GradingNode());
    m_Manager.addNode(g, m_Session);

    // OMG: erase
    INode::ptr n = m_Manager.nodeByName("fr.mikrosimage.dukex.grading");
    if (n.get() != NULL) {
        GradingNode::ptr grading = boost::dynamic_pointer_cast<GradingNode>(n);
    }

    m_Session->startSession("127.0.0.1", 7171, m_RenderWindow->renderWindowID());
    m_statusInfo->setText("Connecting...");

    // Main Timer
    m_timerID = QObject::startTimer(40);
}

//QMenu* UIApplication::createMenu(QObject* _plugin, const QString & _title) {
//    QMenu * m = new QMenu(_title, this);
//    menuBar()->addMenu(m);
//    m_LoadedUIElements.insert(_plugin, m);
//    return m;
//}

bool UIApplication::createWindow(QObject* _plugin, UIWidget* uiwidget, const Qt::DockWidgetArea & _area, const QString & _title) {
    QDockWidget * dockwidget = new QDockWidget(_title, this);
    uiwidget->setParent(dockwidget);
    m_Session->addObserver(uiwidget);
    dockwidget->setWidget(uiwidget);
    addDockWidget(_area, dockwidget);
    m_LoadedUIElements.insert(_plugin, dockwidget);
    return true;
}

//QDeclarativeItem* UIApplication::createQMLWindow(QObject* _plugin, const QUrl &qmlfile, const Qt::DockWidgetArea & _area, const QString & _title) {
//    QDockWidget * dockwidget = new QDockWidget(_title, this);
//    UIView * view = new UIView(dockwidget);
//    QGraphicsScene* scene = new QGraphicsScene();
//    QGraphicsWidget *widget = new QGraphicsWidget();
//    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout();
//    widget->setLayout(layout);
//    scene->addItem(widget);
//    view->setScene(scene);
//
//    //Add the QML snippet into the layout
//    QDeclarativeContext * rootContext = m_Engine.rootContext();
//    QDeclarativeContext * context = new QDeclarativeContext(rootContext);
//    QDeclarativeComponent * c = new QDeclarativeComponent(&m_Engine, qmlfile, view);
//    qDebug() << c->errors();
//    if (!c) {
//        return NULL;
//    }
//    QDeclarativeItem * item = qobject_cast<QDeclarativeItem *> (c->create(context));
//    if (!item)
//        return NULL;
//    QGraphicsLayoutItem* obj = qobject_cast<QGraphicsLayoutItem*> (item);
//    if (!obj)
//        return NULL;
//    layout->addItem(obj);
//
//    //	widget->setGeometry(QRectF(0,0, 400,400));
//    view->setSceneRect(item->childrenRect());
//    view->show();
//    dockwidget->setWidget(view);
//    addDockWidget(_area, dockwidget);
//
//    m_LoadedUIElements.insert(_plugin, dockwidget);
//    return item;
//}

// private
void UIApplication::closeEvent(QCloseEvent *event) {
    m_RenderWindow->close();
    m_Session->stopSession();
    QObject::killTimer(m_timerID);
    m_Manager.clearNodes();
    m_Preferences.saveShortcuts(this);
    m_Preferences.saveFileHistory();
    QMainWindow::closeEvent(event);
    event->accept();
}

// private
void UIApplication::timerEvent(QTimerEvent *event) {
    m_Session->computeInMsg();
    // check connection status
    if (!m_Session->connected()) {
        m_statusInfo->setStyleSheet("QLabel { color : red; }");
        m_statusInfo->setText("Disconnected.");
    } else {
        m_statusInfo->setStyleSheet("QLabel { color : green; }");
        m_statusInfo->setText("Connected.");
    }
    event->accept();
}

// private
void UIApplication::keyPressEvent(QKeyEvent * event) {
    switch (event->key()) {
        case Qt::Key_Escape:
            close();
            break;
        default:
            break;
    }
    event->accept();
}

// private
void UIApplication::dragEnterEvent(QDragEnterEvent *event) {
    event->acceptProposedAction();
}

// private
void UIApplication::dragMoveEvent(QDragMoveEvent *event) {
    event->acceptProposedAction();
}

// private
void UIApplication::dropEvent(QDropEvent *event) {
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        QString text;
        for (int i = 0; i < urlList.size() && i < 32; ++i) {
            open(urlList.at(i).path());
        }
    } else {
        m_statusInfo->setText(tr("Cannot display data"));
    }
    setBackgroundRole(QPalette::Dark);
    event->acceptProposedAction();
}

// private
void UIApplication::resizeCentralWidget(const QSize& resolution) {
    QSize renderRes = m_RenderWindow->size();
    if (resolution == renderRes)
        return;
    QList<int> sizes;
    QObjectList childs = children();
    // Fix size of all children widget
    for (int i = 0; i < childs.size(); ++i) {
        QDockWidget* dock = qobject_cast<QDockWidget*> (childs[i]);
        if (!dock || dock->isFloating())
            continue;
        switch (dockWidgetArea(dock)) {
            case Qt::LeftDockWidgetArea:
            case Qt::RightDockWidgetArea: // Constraint width
                sizes.append(dock->minimumWidth());
                sizes.append(dock->maximumWidth());
                dock->setFixedWidth(dock->width());
                break;
            case Qt::TopDockWidgetArea:
            case Qt::BottomDockWidgetArea: // Constraint height
                sizes.append(dock->minimumHeight());
                sizes.append(dock->maximumHeight());
                dock->setFixedHeight(dock->height());
                break;
            default:
                break;
        }
    }
    // Resize renderer to the new resolution and prevent resize event
    m_RenderWindow->blockSignals(true);
    m_RenderWindow->setFixedSize(resolution);
    m_RenderWindow->blockSignals(false);
    // resize its parent (QMainWindow)
    adjustSize();
    // Allow user to resize renderer
    m_RenderWindow->setMinimumSize(0, 0);
    m_RenderWindow->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    // Restore min & max size of all children widget
    for (int i = 0; i < childs.size(); ++i) {
        QDockWidget* dock = qobject_cast<QDockWidget*> (childs[i]);
        if (!dock || dock->isFloating())
            continue;
        switch (dockWidgetArea(dock)) {
            case Qt::LeftDockWidgetArea:
            case Qt::RightDockWidgetArea:
                dock->setMinimumWidth(sizes.takeFirst());
                dock->setMaximumWidth(sizes.takeFirst());
                break;
            case Qt::TopDockWidgetArea:
            case Qt::BottomDockWidgetArea:
                dock->setMinimumHeight(sizes.takeFirst());
                dock->setMaximumHeight(sizes.takeFirst());
                break;
        }
    }
}

void UIApplication::updateRecentFilesMenu() {
    ui.openRecentMenu->clear();
    for (size_t i = 0; i < m_Preferences.history().size(); ++i) {
        if (m_Preferences.history(i) == "")
            continue;
        boost::filesystem::path fn(m_Preferences.history(i));
        QAction * act = ui.openRecentMenu->addAction(fn.leaf().c_str());
        act->setData(m_Preferences.history(i).c_str());
        connect(act, SIGNAL(triggered()), this, SLOT(openRecent()));
    }
}

// private slot
void UIApplication::open(const QString & _filename) {
    INode::ptr n = m_Manager.nodeByName("fr.mikrosimage.dukex.playlist");
    if (n.get() != NULL) {
        PlaylistNode::ptr p = boost::dynamic_pointer_cast<PlaylistNode>(n);
        if (p.get() != NULL) {
            if (!_filename.isEmpty()) {
                p->open(_filename.toStdString());
                m_Preferences.addToHistory(_filename.toStdString());
                updateRecentFilesMenu();
            }
        }
    }
    firstFrame();
}

// private slot
void UIApplication::open() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), "~", tr("Playlist Files (*.ppl *.ppl2)"));
    open(filename);
}

// private slot
void UIApplication::openRecent() {
    QAction *action = qobject_cast<QAction *> (sender());
    if (action) {
        QString filename = action->data().toString();
        open(filename);
    }
}

// private slot
void UIApplication::playStop() {
    INode::ptr n = m_Manager.nodeByName("fr.mikrosimage.dukex.transport");
    if (n.get() != NULL) {
        TransportNode::ptr t = boost::dynamic_pointer_cast<TransportNode>(n);
        if (t.get() != NULL) {
            if (m_Session->isPlaying())
                t->stop();
            else
                t->play();
        }
    }
}

// private slot
void UIApplication::previousFrame() {
    INode::ptr n = m_Manager.nodeByName("fr.mikrosimage.dukex.transport");
    if (n.get() != NULL) {
        TransportNode::ptr t = boost::dynamic_pointer_cast<TransportNode>(n);
        if (t.get() != NULL) {
            t->previousFrame();
        }
    }
}

// private slot
void UIApplication::nextFrame() {
    INode::ptr n = m_Manager.nodeByName("fr.mikrosimage.dukex.transport");
    if (n.get() != NULL) {
        TransportNode::ptr t = boost::dynamic_pointer_cast<TransportNode>(n);
        if (t.get() != NULL) {
            t->nextFrame();
        }
    }
}

// private slot
void UIApplication::firstFrame() {
    INode::ptr n = m_Manager.nodeByName("fr.mikrosimage.dukex.transport");
    if (n.get() != NULL) {
        TransportNode::ptr t = boost::dynamic_pointer_cast<TransportNode>(n);
        if (t.get() != NULL) {
            t->firstFrame();
        }
    }
}

// private slot
void UIApplication::lastFrame() {
    INode::ptr n = m_Manager.nodeByName("fr.mikrosimage.dukex.transport");
    if (n.get() != NULL) {
        TransportNode::ptr t = boost::dynamic_pointer_cast<TransportNode>(n);
        if (t.get() != NULL) {
            t->lastFrame();
        }
    }
}

// private slot
void UIApplication::previousShot() {
    INode::ptr n = m_Manager.nodeByName("fr.mikrosimage.dukex.transport");
    if (n.get() != NULL) {
        TransportNode::ptr t = boost::dynamic_pointer_cast<TransportNode>(n);
        if (t.get() != NULL) {
            t->previousShot();
        }
    }
}

// private slot
void UIApplication::nextShot() {
    INode::ptr n = m_Manager.nodeByName("fr.mikrosimage.dukex.transport");
    if (n.get() != NULL) {
        TransportNode::ptr t = boost::dynamic_pointer_cast<TransportNode>(n);
        if (t.get() != NULL) {
            t->nextShot();
        }
    }
}

// private slot
void UIApplication::fullscreen() {
    m_RenderWindow->showFullScreen();
}

// private slot
void UIApplication::toggleFitMode() {
    INode::ptr n = m_Manager.nodeByName("fr.mikrosimage.dukex.fit");
    if (n.get() != NULL) {
        FitNode::ptr f = boost::dynamic_pointer_cast<FitNode>(n);
        if (f.get() != NULL) {
            f->toggle();
        }
    }
}

// private slot
void UIApplication::fitToNormalSize() {
    INode::ptr n = m_Manager.nodeByName("fr.mikrosimage.dukex.fit");
    if (n.get() != NULL) {
        FitNode::ptr f = boost::dynamic_pointer_cast<FitNode>(n);
        if (f.get() != NULL) {
            f->fitToNormalSize();
        }
    }
}

// private slot
void UIApplication::fitImageToWindowWidth() {
    INode::ptr n = m_Manager.nodeByName("fr.mikrosimage.dukex.fit");
    if (n.get() != NULL) {
        FitNode::ptr f = boost::dynamic_pointer_cast<FitNode>(n);
        if (f.get() != NULL) {
            f->fitImageToWindowWidth();
        }
    }
}

// private slot
void UIApplication::fitImageToWindowHeight() {
    INode::ptr n = m_Manager.nodeByName("fr.mikrosimage.dukex.fit");
    if (n.get() != NULL) {
        FitNode::ptr f = boost::dynamic_pointer_cast<FitNode>(n);
        if (f.get() != NULL) {
            f->fitImageToWindowHeight();
        }
    }
}

// private slot
void UIApplication::stretchImageToWindow() {
    INode::ptr n = m_Manager.nodeByName("fr.mikrosimage.dukex.fit");
    if (n.get() != NULL) {
        FitNode::ptr f = boost::dynamic_pointer_cast<FitNode>(n);
        if (f.get() != NULL) {
            f->stretchImageToWindow();
        }
    }
}

// private slot
void UIApplication::zoom(double z) {
    INode::ptr n = m_Manager.nodeByName("fr.mikrosimage.dukex.grading");
    if (n.get() != NULL) {
        GradingNode::ptr f = boost::dynamic_pointer_cast<GradingNode>(n);
        if (f.get() != NULL) {
            f->setZoom(z);
        }
    }
}

// private slot
void UIApplication::pan(double x, double y) {
    INode::ptr n = m_Manager.nodeByName("fr.mikrosimage.dukex.grading");
    if (n.get() != NULL) {
        GradingNode::ptr f = boost::dynamic_pointer_cast<GradingNode>(n);
        if (f.get() != NULL) {
            f->setPan(x, y);
        }
    }
}

// private slot
void UIApplication::about() {
    QString msg("DukeX player\n");
    msg += BUILD_INFORMATION;
    QMessageBox::about(this, tr("About DukeX"), msg);
}

// private slot
void UIApplication::aboutPlugins() {
    m_PluginDialog->exec();
}
