/**
 * @file
 *
 * @copyright (C) Picviz Labs 2012-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#include <pvkernel/core/PVDataTreeObject.h>
#include <pvkernel/widgets/PVDataTreeModel.h>

#include <pvkernel/widgets/PVDataTreeMaskProxyModel.h>

#include <QApplication>
#include <QMainWindow>
#include <QDialog>
#include <QVBoxLayout>
#include <QTreeView>

#include <libgen.h>

// Data-tree structure
//

// forward declarations
class A;
class B;
class C;
class D;
class E;

typedef typename PVCore::PVDataTreeObject<D, PVCore::PVDataTreeNoChildren<E>> data_tree_e_t;
class E : public data_tree_e_t
{

  public:
	E(D* d, int i = 0) : data_tree_e_t(d), _i(i) {}

  public:
	virtual ~E() { std::cout << "~E(" << this << ")" << std::endl; }

  public:
	int get_i() const { return _i; }
	void set_i(int i) { _i = i; }

	virtual QString get_serialize_description() const
	{
		return QString("E: ") + QString::number(get_i());
	}

	void serialize(PVCore::PVSerializeObject&, PVCore::PVSerializeArchive::version_t) {}

  private:
	int _i;
};

typedef typename PVCore::PVDataTreeObject<C, E> data_tree_d_t;
class D : public data_tree_d_t
{

  public:
	D(C* c, int i = 0) : data_tree_d_t(c), _i(i) {}

  public:
	virtual ~D() { std::cout << "~D(" << this << ")" << std::endl; }

  public:
	int get_i() const { return _i; }
	void set_i(int i) { _i = i; }

	virtual QString get_serialize_description() const
	{
		return QString("D: ") + QString::number(get_i());
	}

	void serialize(PVCore::PVSerializeObject&, PVCore::PVSerializeArchive::version_t) {}

  private:
	int _i;
};


typedef typename PVCore::PVDataTreeObject<B, D> data_tree_c_t;
class C : public data_tree_c_t
{

  public:
	C(B* b, int i = 0) : data_tree_c_t(b), _i(i) {}

  public:
	virtual ~C() { std::cout << "~C(" << this << ")" << std::endl; }

  public:
	int get_i() const { return _i; }
	void set_i(int i) { _i = i; }

	virtual QString get_serialize_description() const
	{
		return QString("C: ") + QString::number(get_i());
	}

	void serialize(PVCore::PVSerializeObject&, PVCore::PVSerializeArchive::version_t) {}

  private:
	int _i;
};

typedef typename PVCore::PVDataTreeObject<A, C> data_tree_b_t;
class B : public data_tree_b_t
{
	friend class A;

  public:
	B(A* a, int i = 0) : data_tree_b_t(a), _i(i) {}

  public:
	virtual ~B() { std::cout << "~B(" << this << ")" << std::endl; }

  public:
	int get_i() const { return _i; }
	void set_i(int i) { _i = i; }

	virtual QString get_serialize_description() const
	{
		return QString("B: ") + QString::number(get_i());
	}

	void serialize(PVCore::PVSerializeObject&, PVCore::PVSerializeArchive::version_t) {}

  private:
	int _i;
};

typedef typename PVCore::PVDataTreeObject<PVCore::PVDataTreeNoParent<A>, B> data_tree_a_t;
class A : public data_tree_a_t
{

  public:
	A(int i = 0) : data_tree_a_t(), _i(i) {}

  public:
	virtual ~A() { std::cout << "~A(" << this << ")" << std::endl; }

  public:
	int get_i() const { return _i; }
	void set_i(int i) { _i = i; }

	virtual QString get_serialize_description() const
	{
		return QString("A: ") + QString::number(get_i());
	}

  private:
	int _i;
};


typedef typename A::p_type A_p;
typedef typename B::p_type B_p;
typedef typename C::p_type C_p;
typedef typename D::p_type D_p;
typedef typename E::p_type E_p;

typedef PVWidgets::PVDataTreeMaskProxyModel<C> proxy_model_c_t;
typedef PVWidgets::PVDataTreeMaskProxyModel<std::string> proxy_model_d_t;

void usage(char* progname)
{
	std::cerr << "usage: " << basename(progname) << " [1|2|3]" << std::endl;
	std::cerr << "\t1: to display only model view" << std::endl;
	std::cerr << "\t2: to display only proxy view" << std::endl;
	std::cerr << "\t3: to display model view and proxy view" << std::endl;
}

int main(int argc, char** argv)
{
	// Objects, let's create our tree !
	A_p a(new A());
	B_p b1 = a->emplace_add_child(0);
	B_p b2 = a->emplace_add_child(1);
	C_p c1 = b1->emplace_add_child(0);
	C_p c2 = b1->emplace_add_child(1);
	C_p c4 = b2->emplace_add_child(2);
	C_p c5 = b2->emplace_add_child(3);
	D_p d1 = c1->emplace_add_child(0);
	D_p d2 = c1->emplace_add_child(1);
	D_p d4 = c2->emplace_add_child(2);
	D_p d5 = c2->emplace_add_child(3);
	D_p d6 = c4->emplace_add_child(4);
	D_p d7 = c5->emplace_add_child(5);
	E_p e1 = d1->emplace_add_child(0);
	E_p e2 = d1->emplace_add_child(1);

	// Qt app
	QApplication app(argc, argv);

	int what = 3;

	if (argc != 1) {
		what = atoi(argv[1]);
	}
	if (what == 0) {
		usage(argv[0]);
		return 1;
	}

	// Create our model and its view
	PVWidgets::PVDataTreeModel* model = new PVWidgets::PVDataTreeModel(*a);

	QTreeView* view = new QTreeView();
	view->setModel(model);
	view->expandAll();

	QMainWindow* mw = new QMainWindow();
	mw->setCentralWidget(view);
	mw->setWindowTitle("Data Tree - Model");
	mw->show();

	// Create our proxy and its view
	proxy_model_c_t* proxy_c = new proxy_model_c_t();
	proxy_c->setSourceModel(model);

	proxy_model_d_t* proxy_d = new proxy_model_d_t();
	proxy_d->setSourceModel(proxy_c);

	QTreeView* view2 = new QTreeView();
	view2->setModel(proxy_d);
	view2->expandAll();

	QMainWindow* mw2 = new QMainWindow();
	mw2->setWindowTitle("Data Tree - Proxy on class C");
	mw2->setCentralWidget(view2);

	mw2->show();

	return app.exec();
}
