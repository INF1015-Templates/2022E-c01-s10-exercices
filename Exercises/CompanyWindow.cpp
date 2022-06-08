
#include <cstddef>
#include <cstdint>

#include <string>

#include <cppitertools/itertools.hpp>

#include "Company.hpp"
#include "Employee.hpp"
#include "Secretary.hpp"
#include "Manager.hpp"

#include "CompanyWindow.hpp"
#include "ui_CompanyWindow.h"

using namespace std;
using namespace iter;


CompanyWindow::CompanyWindow(QWidget* parent)
: CompanyWindow(make_unique<Company>(), nullptr, parent) { }

CompanyWindow::CompanyWindow(unique_ptr<Company> company, QWidget* parent)
: CompanyWindow(std::move(company), nullptr, parent) { }

CompanyWindow::CompanyWindow(Company* company, QWidget* parent)
: CompanyWindow(nullptr, company, parent) { }

CompanyWindow::CompanyWindow(unique_ptr<Company> companyRes, Company* company, QWidget* parent)
: QMainWindow(parent),
  ui_(make_unique<Ui::CompanyWindow>()),
  companyRessource_(std::move(companyRes)),
  company_(company != nullptr ? company : companyRessource_.get()) {
	ui_->setupUi(this);

	setupUi();
	setupMenu();

	// On connecte les signaux de notre company aux slots créés localement pour agir suite à ces signaux.
	connect(company_, SIGNAL(employeeAdded(Employee*)), this, SLOT(employeeHasBeenAdded(Employee*)));
	connect(company_, SIGNAL(employeeDeleted(Employee*)), this, SLOT(employeeHasBeenDeleted(Employee*)));

	// On remplit nos catégories d'employés.
	employeeCategories_ = {
		&managers_,
		&secretaries_,
		&otherEmployees_,
	};

	loadEmployees();
}

CompanyWindow::~CompanyWindow() { }

void CompanyWindow::setupMenu() {
	ui_->actionExit->setShortcuts(QKeySequence::Quit);
}

void CompanyWindow::setupUi() {
	// Le sélecteur pour filtrer ce que l'on souhaite dans la liste
	connect(ui_->showCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(filterList(int)));

	// La liste des employés
	connect(ui_->employeesList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectEmployee(QListWidgetItem*)));

	// Le bouton pour congédier tout le monde.
	connect(ui_->fireEveryoneButton, SIGNAL(clicked()), this, SLOT(fireEveryone()));

	// Le bouton pour remettre à zéro la vue et créer un nouvel employé
	connect(ui_->hireSomeoneButton, SIGNAL(clicked()), this, SLOT(cleanDisplay()));

	// Boutons radio
	connect(ui_->employeeTypeRadioButtons, SIGNAL(buttonClicked(int)), this, SLOT(changedEmployeeType(int)));

	// Bouton pour congédier la ou les personne(s) sélectionnée(s) dans la liste
	connect(ui_->fireButton, SIGNAL(clicked()), this, SLOT(fireSelected()));

	// Bouton pour embaucher la personne dont on vient d'entrer les informations
	connect(ui_->hireButton, SIGNAL(clicked()), this, SLOT(hireNewEmployee()));

	// Enfin, on met à jour le titre de la fenêtre.
	QString title = "Employee Manager for " + QString(company_->getName().c_str());
	setWindowTitle(title);

	cleanDisplay();
}

void CompanyWindow::loadEmployees() {
	// On s'assure que la liste est vide.
	ui_->employeesList->clear();
	// Puis, pour tous les employés dans Company:
	int max = company_->getNumberEmployees();
	for (int i : range(max)) {
		// On récupère le pointeur vers l'employé.
		Employee* employee = company_->getEmployee(i);
		if (employee == nullptr)
			continue;
		// Et on l'ajoute en tant qu'item de la liste:
		// le nom sera affiché, et le pointeur sera contenu.
		QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(employee->getName()), ui_->employeesList);
		item->setData(Qt::UserRole, QVariant::fromValue<Employee*>(employee));
		item->setHidden(filterHide(employee));
	}
}

bool CompanyWindow::filterHide(Employee* employee) {
	// L'indice 0 est "Show All", donc on ne filtre rien.
	if (currentFilterIndex_ == 0)
		return false;
	// Les indices dans la liste de filtres correspondent aux indices dans notre tableau de catégories.
	return not employeeCategories_[currentFilterIndex_ - 1]->contains(employee);
}

void CompanyWindow::filterList(int index) {
	// On met à jour l'indice de filtre actuel
	currentFilterIndex_ = index;

	// Pour chaque employé:
	for (int i : range(ui_->employeesList->count())) {
		// On prend le pointeur de l'employé à partir de la liste.
		QListWidgetItem *item = ui_->employeesList->item(i);
		Employee* employee = item->data(Qt::UserRole).value<Employee*>();
		// On détermine si l'employé devrait être filtré et on le cache si oui.
		item->setHidden(filterHide(employee));
	}
}

void CompanyWindow::selectEmployee(QListWidgetItem* item) {
	// Quand on sélectionne un employé, il faut afficher ses données.

	Employee* employee = item->data(Qt::UserRole).value<Employee*>();

	// On change l'état des line edits pour qu'ils soient en lecture seule.
	ui_->nameEditor->setReadOnly(true);
	ui_->nameEditor->setText(QString::fromStdString(employee->getName()));

	ui_->salaryEditor->setReadOnly(true);
	ui_->salaryEditor->setText(QString::number(employee->getSalary()));

	// Pour le bonus, on l'affiche seulement si ça s'applique.
	ui_->bonusEditor->setReadOnly(true);
	if (managers_.contains(employee)) {
		ui_->bonusEditor->setDisabled(false);
		ui_->bonusEditor->setText(QString("%1% (included in salary)").arg(((Manager*)employee)->getBonus()));
	} else {
		ui_->bonusEditor->setDisabled(true);
		ui_->bonusEditor->setText("");
	}

	// On coche le bon type d'employé.
	for (auto&& btn : ui_->employeeTypeRadioButtons->buttons()) {
		btn->setDisabled(true);

		bool checked = managers_.contains(employee) and btn->text().endsWith("Manager") or
		               secretaries_.contains(employee) and btn->text().endsWith("Secretary") or
		               otherEmployees_.contains(employee) and btn->text().endsWith("Employee");

		btn->setChecked(checked);
	}

	ui_->fireButton->setDisabled(false);
	ui_->hireButton->setDisabled(true);
}

void CompanyWindow::cleanDisplay() {
	ui_->nameEditor->setReadOnly(false);
	ui_->nameEditor->setText("");

	ui_->salaryEditor->setReadOnly(false);
	ui_->salaryEditor->setText("");

	ui_->bonusEditor->setDisabled(true);
	ui_->bonusEditor->setReadOnly(false);
	ui_->bonusEditor->setText("0");

	for (auto&& btn : ui_->employeeTypeRadioButtons->buttons()) {
		btn->setDisabled(false);
		if (btn->text().endsWith("Employee")) {
			btn->setChecked(true);
		}
	}

	ui_->employeesList->clearSelection();

	ui_->fireButton->setDisabled(true);
	ui_->hireButton->setDisabled(false);

	ui_->nameEditor->setFocus();
}

void CompanyWindow::changedEmployeeType(int index) {
	auto&& foo = ui_->employeeTypeRadioButtons->buttons();
	if (index == -2) {
		ui_->bonusEditor->setDisabled(false);
	} else {
		ui_->bonusEditor->setDisabled(true);
	}
}

void CompanyWindow::fireEveryone() {
	vector<Employee*> toDelete;
	for (int i = 0; i < ui_->employeesList->count(); ++i) {
		QListWidgetItem *item = ui_->employeesList->item(i);
		toDelete.push_back(item->data(Qt::UserRole).value<Employee*>());
	}

	for (Employee* e : toDelete) {
		company_->delEmployee(e);
		for (auto&& cat : employeeCategories_)
			cat->erase(e);
	}
}

void CompanyWindow::fireSelected() {
	vector<Employee*> toDelete;
	for (QListWidgetItem *item : ui_->employeesList->selectedItems()) {
		toDelete.push_back(item->data(Qt::UserRole).value<Employee*>());
	}

	for (Employee* e : toDelete) {
		company_->delEmployee(e);
		for (auto&& cat : employeeCategories_)
			cat->erase(e);
	}
}

void CompanyWindow::hireNewEmployee() {
	// On détermine le bon type d'employé selon les radio buttons.
	string selectedType;
	for (auto&& btn : ui_->employeeTypeRadioButtons->buttons()) {
		if (btn->isChecked()) {
			selectedType = btn->text().toStdString();
			break;
		}
	}
	// On détermine les autres attributs selon les line edits.
	string name = ui_->nameEditor->text().toStdString();
	double salary = ui_->salaryEditor->text().toDouble();
	double bonus = ui_->bonusEditor->text().toDouble();

	// On créé et ajoute l'employé.
	createEmployee(selectedType, name, salary, bonus);
}

void CompanyWindow::employeeHasBeenAdded(Employee* employee) {
	// On ajoute le nouvel employé comme item de la QListWidget
	QListWidgetItem* item = new QListWidgetItem(
		QString::fromStdString(employee->getName()), ui_->employeesList);
	item->setData(Qt::UserRole, QVariant::fromValue<Employee*>(employee));

	// On change la visibilité de notre nouvel employé selon
	// le filtre actuel
	item->setHidden(filterHide(employee));
}

void CompanyWindow::employeeHasBeenDeleted(Employee* e) {
	// On cherche dans notre QlistWidget l'employé pour lequel le
	// signal a été envoyé, afin de l'en retirer
	for (int i = 0; i < ui_->employeesList->count(); ++i) {
		QListWidgetItem *item = ui_->employeesList->item(i);
		Employee* employee = item->data(Qt::UserRole).value<Employee*>();
		if (employee == e) {
			// delete sur un QlistWidget item va automatiquement le retirer de la liste
			delete item;
			// Si l'employé faisait partie de ceux créés localement, on veut le supprimer.
			auto it = std::find_if(added_.begin(), added_.end(), [&] (auto& arg) { return arg.get() == e; });
			if (it != added_.end()) {
				added_.erase(it);
			}
			// L'employé ne devrait être qu'une fois dans la liste...
			break;
		}
	}
	// On remet à zéro l'affichage de la colonne de gauche étant
	// donné que les employés sélectionnés ont été supprimés
	cleanDisplay();
}

Employee* CompanyWindow::createEmployee(const string& type, const string& name, double salary, double bonus) {
	unique_ptr<Employee> newEmployee;

	// On crée selon le type spécifié.
	if (type.ends_with("Manager")) {
		auto man = make_unique<Manager>(name, salary, bonus);
		managers_.insert(man.get());
		newEmployee = std::move(man);
	} else if (type.ends_with("Secretary")) {
		auto sec = make_unique<Secretary>(name, salary);
		secretaries_.insert(sec.get());
		newEmployee = std::move(sec);
	} else {
		newEmployee = make_unique<Employee>(name, salary);
		otherEmployees_.insert(newEmployee.get());
	}

	// On ajoute le nouvel employé créé à la company
	company_->addEmployee(newEmployee.get());
	// Mais on le stocke aussi localement pour pouvoir le supprimer plus tard
	added_.push_back(std::move(newEmployee));

	return added_.back().get();
}


Q_DECLARE_METATYPE(Employee*)

