#include <QVector>
#include <QString>
#include <QSet>
#include <QObject>
#include <QMap>
#include "calculator.h"



/// pre declare something
double operate(double a,QString theta,double b);
bool isnumber(QString s);
QString Precede(QString a,QString b);
////


const QString treeNode::CUSTOMIZEFUNCTION = "customize function";
const QString treeNode::EMBEDDEDFUNCTION = "embedded function";
const QString treeNode::CONSTVALUE = "const value";
const QString treeNode::VARIABLE = "variable";
const QString treeNode::OPTR = "optr";
const QString treeNode::INBACKET = "inbacket";
const QSet<QString> Calculator::optrset = {"*","/","+","-","(",")",",","^","="};
const QSet<QString> Calculator::funcset = {"sin","cos","tan","floor","abs","sqrt",
           "acos","asin","atan","ceil","abs","ln","exp",
          "acosh","asinh","atanh","cbrt","ceil","log",
          "round","sec","max","min","mod"};

Calculator::Calculator(QString expression,QMap<QString,double>*value_variable = nullptr,QMap<QString,QString>*functions = nullptr)
{
    this->expression = expression;
    this->value_variable=value_variable;
    this->functions = functions;
    headnode = new treeNode(treeNode::CUSTOMIZEFUNCTION,"");

}
//词法分析，分割表达式成词
//结果输出到splitresult列表，第一列是放词的内容，第二列是词的类型
QVector<QVector<QString>> Calculator::split(QString expression,bool updatevariable = true)
{
    if(status<1)return {};

    splitresult = {};

    //如果是空就返回，会在上一个update函数，使窗口中对应显示表达式结果的标签不显示东西
    if(expression=="")
    {
        status = -1;
        return {};
    }

    //状态转换图，初始的状态是零
    //做法是先列出有几种状态，比如从零开始，如果下一个字符是字母，就去到状态一
    //如果下一个字符是数字，就去到状态二
    //那么状态一的结果是一串数字，是一个常数变量
    //状态二的结果是一串字母可能穿插着数字，那么可能是函数名或者变量名
    double state = 0;

    //
    QString currentstring = "";
    for(int i = 0;i<expression.size();i++)
    {

        QChar character = expression[i];
        if(character != '*'&&character != '/'&&character != '+'&&character != '('&&character != '-'
                &&character != ')'&&character != ','&&character != '^'&&character != ' '&&character != '.'&&character != '=')
        {
            if(!((character>='a'&&character<='z')||(character>='A'&&character<='Z')||(character>='0'&&character<='9')))
            {

                status = -1;
                return {};
            }
        }
        if(character==' ')
        {
            if(currentstring=="")continue;
            splitresult.append({currentstring});
            currentstring="";
            state = 0;
            continue;
        }
        if(character == '*'||character == '/'||character == '+'||character == '('||
                character == ')'||character == ','||character == '^'||character == '=')
        {
            if(currentstring!="")splitresult.append({currentstring});
            currentstring="";
            splitresult.append({character});
            state = 0;
            continue;
        }

        if(state==0&&character=='-')
        {
            if(splitresult.size()==0||splitresult.back()[0]=="(")
            {
                currentstring+=character;
                state = 1;
                continue;
            }
            else
            {
                splitresult.push_back({"-"});
                continue;
            }
        }
        if(state==0&&(character>='0'&&character<='9'))
        {
            currentstring+=character;
            state = 1;
            continue;

        }
        if(state==0&&((character>='a'&&character<='z')||(character>='A'&&character<='Z')||character=='_'))
        {
            currentstring+=character;
            state = 2;
            continue;
        }
        if(state ==2)
        {
            if((character>='0'&&character<='9')||((character>='a'&&character<='z')||(character>='A'&&character<='Z')||character=='_'))
            {
                currentstring+=character;
            }
            else
            {
                splitresult.append({currentstring});
                currentstring="";
                state = 0;
                i--;
            }
            continue;

        }
        if(state ==3)
        {
            if(character>='0'&&character<='9')
            {
                currentstring+=character;
            }

            else
            {
                splitresult.append({currentstring});
                currentstring="";
                state = 0;
                i--;
            }
            continue;
        }
        if(state ==1)
        {
            if(character>='0'&&character<='9')
            {
                currentstring+=character;
            }
            else if(character=='.')
            {
                currentstring+=character;
                state = 3;
            }
            else
            {
                splitresult.append({currentstring});
                currentstring="";
                state = 0;
                i--;
            }
            continue;
        }
    }

    splitresult.append({currentstring});


    //分类这个词，看他属于哪个类（自定义函数、嵌入函数、变量）
    for(QVector<QString> &s:splitresult)
    {
        QString symbol = s[0];
        bool in = isnumber(symbol);

        if(funcset.find(symbol)!=funcset.end())
        {
            s.append(treeNode::EMBEDDEDFUNCTION);
            continue;
        }
        if(symbol=="f"||symbol=="g"||symbol=="h")
        {
            s.append(treeNode::CUSTOMIZEFUNCTION);
            continue;
        }
        if(symbol=="a"||symbol=="b"||symbol=="c")
        {
            s.append(treeNode::VARIABLE);
            continue;
        }

        if(optrset.find(symbol)!=optrset.end())
        {
            s.append(treeNode::OPTR);
            continue;
        }
        if(in)
        {
            s.append(treeNode::CONSTVALUE);
            continue;
        }
        if(updatevariable==true&& value_variable->find(symbol)==value_variable->end())
        {
            double randval = (rand()%200000-100000)/5000;
            value_variable->insert(symbol,randval);

            s.append(treeNode::VARIABLE);
            continue;
        }
        s.append(treeNode::VARIABLE);
    }
    return splitresult;
}
//做语法树
//parent是双亲结点的指针，index指现在的词法分析结果列表的下标
void Calculator::buildtree(treeNode*parent,int& index,bool isfunction)
{
    if(status<1)return;
    //两分支
    //一个是把函数的参数加入到函数结点的儿子结点列表
    //另一个就像编译原理的做语法树
    if(isfunction == false)
    {

        for(int& i = index;i<splitresult.size();i++)
        {
            //
            QVector<QString>s = splitresult[i];
            if(s[0]==""||s[0]==" ")continue;
            if(s[1]!=treeNode::OPTR&&parent->childs.size()>=1&&splitresult[i-1][1]!=treeNode::OPTR)
            {
                parent->addchild(new treeNode(treeNode::OPTR,"*"));
            }
            if(s[1]!=treeNode::OPTR&&parent->childs.size()>=1&&splitresult[i-1][1]!=treeNode::OPTR)
            {
                parent->addchild(new treeNode(treeNode::OPTR,""));
            }
            //如果词属于变量
            if(s[1]==treeNode::VARIABLE)
            {
                //那么新建一个结点，把词的信息保存到结点，添加到双亲结点的儿子结点指针列表中


                treeNode*newnode = new treeNode(treeNode::VARIABLE,s[0]);
                parent->addchild(newnode);
                parent->content+=s[0];
                continue;
            }
            //如果词是常量
            if(s[1]==treeNode::CONSTVALUE)
            {
                //那么新建一个结点，把词的信息保存到结点，添加到双亲结点的儿子结点指针列表中
                QString hou = s[0];
                if(s[0].size()>0&&s[0][0]=='-'&&(parent->childs.size()>0&&splitresult[i-1][1]!=treeNode::OPTR))
                {
                    treeNode*newnode = new treeNode(treeNode::OPTR,"-");
                    if(s[0].size()>1)
                        hou = s[0].mid(1);
                    parent->addchild(newnode);
                }
                treeNode*newnode = new treeNode(treeNode::CONSTVALUE,hou);
                parent->addchild(newnode);
                parent->content+=s[0];
                continue;
            }
            //如果词是嵌入的函数
            if(s[1]==treeNode::EMBEDDEDFUNCTION)
            {
                //那么新建一个结点，把词的信息保存到结点，添加到双亲结点的儿子结点指针列表中
               treeNode*newnode = new treeNode(treeNode::EMBEDDEDFUNCTION,s[0]);
                parent->addchild(newnode);
                buildtree(newnode,++i,true);
                parent->content+=s[0];
                continue;
            }

            //如果词是自定义的函数
            if(s[1]==treeNode::CUSTOMIZEFUNCTION)
            {
               treeNode*newnode = new treeNode(treeNode::CUSTOMIZEFUNCTION,s[0]);
                parent->addchild(newnode);
                buildtree(newnode,++i,true);
                parent->content+=s[0];
                continue;
            }

            //如果词是左括号
            if(s[0]=="(")
            {
                //
                //那么新建一个结点，把词的信息保存到结点，添加到双亲结点的儿子结点指针列表中
               treeNode*newnode = new treeNode(treeNode::INBACKET,"");
                parent->addchild(newnode);

                //然后递归，传入的参数是1这个新建的结点，下一个位置的下标，不是函数
                buildtree(newnode,++i,false);
                parent->content+=s[0];
                continue;
            }
            if(s[0]==")"||s[0]==",")
            {
                return;
            }

            //如果词是运算符
            if(s[1]==treeNode::OPTR)
            {
                //那么新建一个结点，把词的信息保存到结点，添加到双亲结点的儿子结点指针列表中
               treeNode*newnode = new treeNode(treeNode::OPTR,s[0]);
                parent->addchild(newnode);
                parent->content+=s[0];

            }

        }
    }
    else
    {
        if(index<splitresult.size()&&splitresult[index][0]!="(")
        {
            //函数名的下一个应该是左括号，否则错误。
            //QMessageBox::warning(nullptr,"输入错误","没有输入参数到函数 "+parent->content);
            status = -1;
            return;
        }

        for(;index<splitresult.size();)
        {
            if(splitresult[index][0]==")")return;
            treeNode*newnode = new treeNode(treeNode::INBACKET,"");
            parent->addchild(newnode);
            buildtree(newnode,++index,false);
        }
    }
}

double Calculator::calculate(treeNode *parentnode)
{

    if(status<1)return status;
    parentnode->addchild(new treeNode(treeNode::OPTR,"#"));


    //opndstack是放操作数的栈
    QVector<float>opndstack;
    //放运算符的栈
    QVector<QString>optrstack={"#"};

    //先把儿子结点列表和它的大小保存起来，避免重复地调用
    QVector<treeNode*>childs = parentnode->childs;
    double size = childs.size();
    for(int i =0;i<size && optrstack.size()>0;i++)
    {

        treeNode*child = childs[i];
        //如果是空的就跳过
        if(child->content==""||child->content==" ")continue;

        //如果是运算符，
        if(child->type==treeNode::OPTR)
        {
            //比较运算符栈顶端的运算符和输入的运算符的优先级
            QString compare = Precede(optrstack.last() ,child->content);
            //如果是大于
            if(compare==">")
            {
                //如果操作树栈的长度小于2，出错
                if(opndstack.size()<2)
                {
                    {status = -1;return status;}
                }

                //弹出操作数栈顶端2个操作数和运算符栈顶端的1个运算符
                QString optr = optrstack.last();
                optrstack.pop_back();
                double b = opndstack.last();
                opndstack.pop_back();
                double a = opndstack.last();
                opndstack.pop_back();
                //运算之后，把结果推入到操作数栈
                opndstack.push_back(operate(a,optr,b));
                //下一次继续要这个优先级高的运算符
                i--;
            }
            //如果等于
            //就弹出运算符栈顶端的运算符
            else if(compare=="=")
            {
                optrstack.pop_back();
            }
            //如果等于，就把运算符推入到运算符栈
            else if(compare=="<")
            {
                optrstack.push_back(child->content);
            }
        }

        //如果是操作数
        else
        {
            //如果是变量、常数、函数，就用它们的成员函数getval
            double result  = getNodeValue(child);
            if (status==-1)
            {
                return status;
            }
            opndstack.push_back(result);

        }
    }
    //最后如果操作数栈的长度是1，就返回这个栈顶的操作数
    if(opndstack.size()==1)
    {
        return opndstack.back();

    }
    else
    {
        status = -1;
        return status;
    }

}

double Calculator::solve()
{
    split(expression,true);
    if(status<1)return status;

    int index = 0;
    buildtree(headnode,index,false);
    if(status<1)return status;

    double ans = calculate(headnode);
    if(status<1)return status;

    return ans;
}

double Calculator::getNodeValue(treeNode* node)
{
    //枚举分5类

    //如果词是常量数字
    if(node->type==treeNode::CONSTVALUE)
    {
        //用Qt的toFloat函数
        return node->content.toDouble();
    }

    //如果词是变量
    if(node->type ==treeNode::VARIABLE)
    {
        //去变量列表去找
        return value_variable->operator[](node->content);
    }


    if(node->type ==treeNode::VARIABLE)
    {
        if(value_variable==nullptr)
        {
            status = -1;
            return status;
        }
        //去变量列表去找
        return Calculator::calculate(node);
    }
    //如果词是自定义函数的函数名
    //想法是切出函数中等号右边的表达式
    //然后把这个表达式的变量换成传入的参数
    //然后计算，返回计算的值
    if(node->type ==treeNode::CUSTOMIZEFUNCTION)
    {
        if(functions==nullptr)
        {
            status =-1;
            return status;
        }
        double ans;


        //用来保存词法分析的结果
        QVector<QVector<QString>>sr = split(functions->operator[](node->content),false);
        //等号分成左右两部分
        QVector<QVector<QString>>splitleft;
        QVector<QVector<QString>>splitright;
        //如果ui中那个行编辑内容是空，就什么都不做
        if(sr.size()==1)
        {
            status = -1;
        }
        //用等号分割函数的表达式
        int i =0;
        for(;i<sr.size();i++)
        {
            if(sr[i][0]=="=")break;
            splitleft.push_back(sr[i]);
        }
        i++;
        if(i>=sr.size())
        {
            status = -1;
            return status;
        }
        for(;i<sr.size();i++)
        {
            splitright.push_back(sr[i]);
        }

        //对左边的进行分析，左边就是f(x)之类的
        treeNode*node0 = new treeNode(treeNode::INBACKET,"");
        int index = 0;
        buildtree(node0,index,false);
        if(node->childs.size()!=node0->childs[0]->childs.size())
        {
            status = -1;
            return status;
        }
        //然后映射
        QMap<QString,QString>m;
        for(int i = 0;i<node0->childs[0]->childs.size();i++)
        {
            m[node0->childs[0]->childs[i]->content]=node->childs[i]->content;
        }
        //然后做出映射后的新的表达式，就是从原来函数右边的那个表达式，改变它的参数名
        QString newcode = " ";
        for(int i =0;i<splitright.size();i++)
        {
            QString p = splitright[i][0];
            if(m.find(p)!=m.end())
            {
                p = m[p];
            }
            newcode+=" "+p+" ";
        }
        newcode+=" ";
        //计算这个新表达式的值
        sr = split(newcode,false);
        treeNode*node1 = new treeNode(treeNode::INBACKET,"");
        int index1 = 0;
        buildtree(node1,index1,false);
        ans = calculate(node1);
        return ans;
    }

    //如果是词是嵌入函数的函数名
    if(node->type ==treeNode::EMBEDDEDFUNCTION)
    {
        if(node->childs.size()<1)
        {
            status = -1;
            return status;
        }
        //一个个写
        if(node->content=="sin")return sinf(calculate(node));
        if(node->content=="cos")return cosf(calculate(node));
        if(node->content=="tan")return tanf(calculate(node));
        if(node->content=="asin")return asinf(calculate(node));
        if(node->content=="acos")return acosf(calculate(node));
        if(node->content=="atan")return atanf(calculate(node));
        if(node->content=="acosh")return acoshf(calculate(node));
        if(node->content=="asinh")return asinhf(calculate(node));
        if(node->content=="atanh")return atanhf(calculate(node));
        if(node->content=="sqrt")return sqrtf(calculate(node));
        if(node->content=="cbrt")return cbrtf(calculate(node));
        if(node->content=="ceil")return ceil(calculate(node));
        if(node->content=="erf")return erf(calculate(node));
        if(node->content=="erfc")return erfcl(calculate(node));
        if(node->content=="exp")return exp2f(calculate(node));
        if(node->content=="expm1")return expm1f(calculate(node));
        if(node->content=="round")return roundf(calculate(node));
        if(node->content=="ln")return qLn(calculate(node));
        if(node->content=="abs")return qAbs(calculate(node));
        if(node->content=="sec")return pow(cosf(calculate(node)),-1);
        if(node->content=="floor")return floor(calculate(node));
        if(node->content=="log")
        {
            if(node->childs.size()<2){status = -1; return status;}
            return qLn(calculate(node->childs[1]))/qLn(calculate(node->childs[0]));
        }

        if(node->content=="max")
        {
            double size = node->childs.size();
            if(size<1){status = -1; return status;}
            double ret = calculate(node->childs[0]);
            for(int i =1;i<size;i++)
            {
                ret = qMax(ret,calculate(node->childs[i]));
            }
            return ret;
        }
        if(node->content=="min")
        {
            double size = node->childs.size();
            if(size<1){status = -1; return status;}
            double ret = calculate(node->childs[0]);
            for(int i =1;i<size;i++)
            {
                ret = qMin(ret,calculate(node->childs[i]));
            }
            return ret;
        }
        if(node->content=="mod")
        {
            if(node->childs.size()<2){status = -1; return status;}
            int a = calculate(node->childs[0]);
            int b = calculate(node->childs[1]);
            if(b == 0)
            {
                status = -1;
                return status;
            }
            return a % b;
        }
    }
    if(node->type ==treeNode::INBACKET)
    {
        return getNodeValue(node->childs[0]);
    }
    status = -1;
    return status;
}



//判断一个字符串里的字符是不是属于0123456789
//如果都是就返回true否则返回false
bool isnumber(QString s)
{

    for(int i =0;i<s.size(); i++)
    {
        if(s[i]=='.'||s[i]=='-')continue;
        if(s[i]<'0'||s[i]>'9')return false;
    }
    return true;
}
//操作两个数，返回操作的结果
double operate(double a,QString theta,double b)
{
    if(theta=="+")
        return a+b;
    else if(theta=="-")
        return a-b;
    else if(theta=="*")
        return a*b;
    else if(theta=="/")
        return a/b;
    else if(theta=="^")
        return qPow(a,b);
    return -1;
}
QString Precede(QString a,QString b)
{
    if(a=='+'||a=='-')
    {
        if(b=='+'||b=='-'||b==')'||b=='#')
            return ">";
        else
            return "<";
    }
    else if(a=='*'||a=='/')
    {
        if(b=='('||b=='^')
            return "<";
        else
            return ">";
    }
    else if(a=='(')
    {
        if(b=='+'||b=='-'||b=='*'||b=='/'||b=='(')
            return "<";
        else if(b==')')
            return "=";
        else
        {
            printf("输入有误");
            return 0;
        }
    }
    else if(a==')')
    {
        if(b=='+'||b=='-'||b=='*'||b=='/'||b==')'||b=='#')
            return ">";
        else if(b=='(')
        {
            printf("输入有误");
            return 0;
        }
    }
    else if(a=="^")
    {
        if(b=='(')
            return "<";
        else
            return ">";
    }
    else if(a=='#')
    {
        if(b==')')
        {
            printf("输入有误");
            return 0;
        }
        else if(b=='#')
            return "=";
        else
            return "<";
    }
    return "";
}
void Calculator::destroytree(treeNode*p)
{
    if(p==nullptr)return;
    for(treeNode* n:p->childs)
    {
        destroytree(n);
    }
    delete p;
}
Calculator::~Calculator()
{
    destroytree(headnode);
}
