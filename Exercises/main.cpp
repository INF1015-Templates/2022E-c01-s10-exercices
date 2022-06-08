
#include <cstddef>
#include <cstdint>

#include <QApplication>

#include "CounterWindow.hpp"
#include "CompanyWindow.hpp"

using namespace std;


void preAddEmployees(CompanyWindow& window) {
	window.createEmployee("Employee",  "Emp A", 42);
	window.createEmployee("Employee",  "Emp B", 69);
	window.createEmployee("Secretary", "Sec A", 420);
	window.createEmployee("Secretary", "Sec B", 1337);
	window.createEmployee("Manager",   "Mngr A", 0xDEAD);
	window.createEmployee("Manager",   "Mngr B", 0xBEEF);
}


int main(int argc, char* argv[]) {
	QApplication a(argc, argv);

	//CounterWindow w;

	CompanyWindow w(make_unique<Company>("Chosson & Friends LLC", "theRealChosson"));
	preAddEmployees(w);

	w.show();
	return a.exec();
}
