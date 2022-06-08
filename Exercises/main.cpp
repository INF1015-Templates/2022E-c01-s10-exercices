
#include <cstddef>
#include <cstdint>

#include <QApplication>

#include "CounterWindow.hpp"
#include "CompanyWindow.hpp"

using namespace std;


void preAddEmployees(CompanyWindow& window) {
	window.createEmployee("Employee",  "Mohamed Doyon", 42);
	window.createEmployee("Employee",  "Kevin Barrette", 69);
	window.createEmployee("Secretary", "Alyson Nolin", 420);
	window.createEmployee("Secretary", "Robert Thomas", 1337);
	window.createEmployee("Manager",   "David Ducharme", 0xDEAD, 10);
	window.createEmployee("Manager",   "Claude Hernandez", 0xBEEF, 15);
}


int main(int argc, char* argv[]) {
	QApplication a(argc, argv);

	//CounterWindow w;

	CompanyWindow w(make_unique<Company>("Chosson & Friends LLC", "theRealChosson"));
	preAddEmployees(w);

	w.show();
	return a.exec();
}
