#ifndef GUI_GUI_MAIN_WINDOW_H
#define GUI_GUI_MAIN_WINDOW_H

#include <QMainWindow>

namespace gui {

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
};


} // namespace gui
#endif // GUI_GUI_MAIN_WINDOW_H
