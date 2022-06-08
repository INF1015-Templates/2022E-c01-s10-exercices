
#include <cstddef>
#include <cstdint>

#include <QApplication>

#include "CounterWindow.hpp"
#include "CompanyWindow.hpp"

using namespace std;


int main(int argc, char* argv[]) {
	QApplication a(argc, argv);
	//CounterWindow w;
	CompanyWindow w(make_unique<Company>("Chosson & Friends LLC", "theRealChosson"));
	w.show();
	return a.exec();
}
