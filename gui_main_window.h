/** @file
  @author Ralph Tandetzky
  @date 2 Apr 2014
*/

#pragma once
#include <QMainWindow>
#include <memory>

namespace gui {

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget * parent = nullptr);
    ~MainWindow();
    
private slots:
    void parse();
    void runNextOptimization();

private:
    struct Impl;
    std::unique_ptr<Impl> m;
};

} // namespace gui
