// QmitkFctMediator.cpp: implementation of the QmitkFctMediator class.
//
//////////////////////////////////////////////////////////////////////

#include "QmitkFctMediator.h"

#include <qwidgetstack.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qtoolbar.h>
#include <qaction.h>
#include <qabstractlayout.h> 
#include <qlayout.h>

#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QmitkFctMediator::QmitkFctMediator(QObject *parent, const char *name) : QObject(parent, name), 
    m_MainStack(NULL), m_ControlStack(NULL), m_ButtonMenu(NULL), m_ToolBar(NULL), m_DefaultMain(NULL), 
    m_FunctionalityActionGroup(NULL),
    m_NumOfFuncs(0), m_CurrentFunctionality(0)
{

}

QmitkFctMediator::~QmitkFctMediator()
{
/*	QmitkFunctionality *functionality;
	for ( functionality=qfl.first(); functionality != 0; functionality=qfl.next() )
		delete functionality;
	qfl.clear();*/
}

void QmitkFctMediator::initialize(QWidget *aLayoutTemplate)
{
    if(aLayoutTemplate==NULL)
        return;

    QWidget *w;

    if((w=static_cast<QWidget*>(aLayoutTemplate->child("MainParent", "QWidget")))!=NULL)
    {
		QHBoxLayout* hlayout=new QHBoxLayout(w);
        hlayout->setAutoAdd(true);
        m_MainStack = new QWidgetStack(w, "QmitkFctMediator::mainStack");

        m_DefaultMain = new QWidget(m_MainStack,"QmitkFctMediator::m_DefaultMain");
        m_MainStack->addWidget(m_DefaultMain);

        connect( m_MainStack, SIGNAL(aboutToShow(int)), this, SLOT(functionalitySelected(int)) );
    }

    if((w=static_cast<QWidget*>(aLayoutTemplate->child("ControlParent", "QWidget")))!=NULL)
    {
		QHBoxLayout* hlayout=new QHBoxLayout(w);
        hlayout->setAutoAdd(true);
        m_ControlStack = new QWidgetStack(w, "QmitkFctMediator::controlStack");
    }

    if((w=static_cast<QWidget*>(aLayoutTemplate->child("ButtonMenuParent", "QButtonGroup")))!=NULL)
    {
        m_ButtonMenu=static_cast<QButtonGroup*>(w);
	    m_ButtonMenu->setExclusive(true);
        if(m_ButtonMenu!=NULL)
            connect( m_ButtonMenu, SIGNAL(clicked(int)), this, SLOT(raiseFunctionality(int)) );
    }

    if((w=static_cast<QWidget*>(aLayoutTemplate->child("ToolBar", "QWidget")))!=NULL)
    {
        m_ToolBar=w;

        m_FunctionalityActionGroup = new QActionGroup( this );
        m_FunctionalityActionGroup->setExclusive( TRUE );
    
        connect( m_FunctionalityActionGroup, SIGNAL( selected( QAction* ) ), this, SLOT( raiseFunctionality( QAction* ) ) );
    }
}

bool QmitkFctMediator::addFunctionality(QmitkFunctionality * functionality)
{
	if(functionality==NULL)
	{
        if(m_ButtonMenu!=NULL)
        {
            QLayout * menuLayout = m_ButtonMenu->layout();
		    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
		    menuLayout->addItem( spacer );
        }
	    return true;
	}

	qfl.append(functionality);

	connect(functionality, SIGNAL(availabilityChanged()), this, SLOT(checkAvailability()));

    QPushButton* funcButton(NULL); 
    if(m_ButtonMenu!=NULL)
    {
        QLayout * menuLayout = m_ButtonMenu->layout();
        if(menuLayout==NULL)
        {
            menuLayout = new QVBoxLayout(m_ButtonMenu,0,10);
            menuLayout->setSpacing( 2 );
            menuLayout->setMargin( 11 );
        }
	    
        char number[20]; sprintf(number,"%d",m_NumOfFuncs);

	    funcButton = new QPushButton(functionality->getFunctionalityName(), m_ButtonMenu, number);
	    funcButton->setToggleButton(true);
	    menuLayout->addItem(new QWidgetItem(funcButton));

        if((qfl.count()>1) && (functionality->isAvailable()==false))
		    funcButton->setEnabled(false);
    }

    QAction* action=NULL;
    if(m_FunctionalityActionGroup!=NULL)
    {

        action = functionality->createAction(m_FunctionalityActionGroup);
        if(action!=NULL)
        {
            action->setToggleAction( true );
            if(m_ToolBar!=NULL)
                action->addTo( m_ToolBar );
        }
    }
    qal.append(action);

    if(m_MainStack!=NULL)
    {
        QWidget * mainWidget = functionality->createMainWidget(m_MainStack);
        if(mainWidget!=NULL)
            m_MainStack->addWidget(mainWidget, m_NumOfFuncs+1);
        else
            m_MainStack->addWidget(new QWidget(m_MainStack, "QmitkFctMediator::dummyMain"), m_NumOfFuncs+1);
    }
    if(m_ControlStack!=NULL)
    {
        QWidget * controlWidget = functionality->createControlWidget(m_ControlStack);
        if(controlWidget!=NULL)
            m_ControlStack->addWidget(controlWidget, m_NumOfFuncs);
        else
            m_ControlStack->addWidget(new QWidget(m_ControlStack, "QmitkFctMediator::dummyControl"), m_NumOfFuncs);
    }

	if(m_NumOfFuncs==0)
	{
		if(funcButton!=NULL)
            funcButton->setOn(true);
        if(action!=NULL)
            action->setOn(true);
        if(m_ControlStack!=NULL)
		    m_ControlStack->raiseWidget(0);
        if((m_MainStack!=NULL) && (strcmp(m_MainStack->widget(0+1)->name(),"QmitkFctMediator::dummyMain")!=0))
		    m_MainStack->raiseWidget(0+1);
	}

    functionality->createConnections();

    ++m_NumOfFuncs;

	return true;
}

QWidget * QmitkFctMediator::getControlParent()
{
	return m_ControlStack;
}

QWidget * QmitkFctMediator::getMainParent()
{
	return m_MainStack;
}

QButtonGroup * QmitkFctMediator::getButtonMenu()
{
	return m_ButtonMenu;
}

QWidget * QmitkFctMediator::getToolBar()
{
	return m_ToolBar;
}

QWidget * QmitkFctMediator::getDefaultMain()
{
	return m_DefaultMain;
}

void QmitkFctMediator::selecting(int id)
{
	if(id!=m_CurrentFunctionality)
		qfl.at(m_CurrentFunctionality)->deactivated();
}

void QmitkFctMediator::functionalitySelected(int id)
{
    if(id==0) return;
	m_CurrentFunctionality=id-1; //kommt von m_MainStack und der hat einen Eintrag mehr (wg. m_DefaultMain)
	qfl.at(m_CurrentFunctionality)->activated();		
}

QmitkFunctionality* QmitkFctMediator::getFunctionalityByName(const char *name)
{
	QmitkFunctionality *functionality;
	for ( functionality=qfl.first(); functionality != 0; functionality=qfl.next() )
		if(strcmp(functionality->getFunctionalityName().ascii(),name)==0)
			return functionality;
	return NULL;
}

void QmitkFctMediator::raiseFunctionality(int id)
{
    QAction *action;
    action=qal.at(id);
    if((action!=NULL) && (action->isOn()==false))
    {
        action->setOn(true);
        return; //we will come into this method again ...
    }

    if(m_ButtonMenu!=NULL)
	    ((QButtonGroup*)m_ButtonMenu)->setButton(id);
    if(m_ToolBar!=NULL)
	    ((QButtonGroup*)m_ToolBar)->setButton(id);

	selecting(id);
	m_ControlStack->raiseWidget(id);
    if(strcmp(m_MainStack->widget(id+1)->name(),"QmitkFctMediator::dummyMain")!=0)
	    m_MainStack->raiseWidget(id+1);
    else
        m_MainStack->raiseWidget(0);
	functionalitySelected(id+1);
}

void QmitkFctMediator::raiseFunctionality(QAction* action)
{
    int id=qal.find(action);
    if(id>=0)
      raiseFunctionality(id);
}

void QmitkFctMediator::raiseFunctionality(QmitkFunctionality* aFunctionality)
{
	QmitkFunctionality *functionality;
	int id=0;
	for ( functionality=qfl.first(); functionality != 0; functionality=qfl.next(),++id )
		if(functionality==aFunctionality)
		{
			raiseFunctionality(id);
			return;
		}
}

void QmitkFctMediator::checkAvailability()
{
	QmitkFunctionality *functionality;
	int id;
    if(m_ButtonMenu!=NULL)
	    for ( functionality=qfl.first(), id=0; functionality != 0; functionality=qfl.next(),++id )
		    ((QButtonGroup*)m_ButtonMenu)->find(id)->setEnabled(functionality->isAvailable());
}

void QmitkFctMediator::hideControls(bool hide)
{
	if(m_ControlStack==NULL) return;

    QWidget *controlStackParent = dynamic_cast<QWidget*>(m_ControlStack->parent());
	if(controlStackParent==NULL) return;

	if(hide)
		controlStackParent->hide();
	else
		controlStackParent->show();
}

void QmitkFctMediator::hideMenu(bool hide)
{
	if(m_ButtonMenu=NULL) return;

	if(hide)
		m_ButtonMenu->hide();
	else
		m_ButtonMenu->show();
}

