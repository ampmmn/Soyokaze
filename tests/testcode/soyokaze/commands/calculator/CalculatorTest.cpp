#include "stdafx.h"
#include "commands/calculator/Calculator.h"

using launcherapp::commands::calculator::Calculator;

TEST(CalculatorTest, DoesNotEvaluateWhenBothFeaturesAreDisabled)
{
	Calculator calculator;
	calculator.UseStandardEvaluate(false);
	calculator.UseMathypadEvaluate(false);

	CString result;
	EXPECT_FALSE(calculator.Evaluate(_T("1 + 1"), result));
}

TEST(CalculatorTest, FormatsBinaryResult)
{
	EXPECT_EQ(
		_T("0b 11 10010010 11111010 (bit=17,16,15,12,9,7,6,5,4,3,1)"),
		Calculator::FormatBinaryResult(_T("0b111001001011111010")));
	EXPECT_EQ(
		_T("-0b 11 10010010 11111010 (bit=17,16,15,12,9,7,6,5,4,3,1)"),
		Calculator::FormatBinaryResult(_T("-0b111001001011111010")));
	EXPECT_EQ(_T("0b 0"), Calculator::FormatBinaryResult(_T("0b0")));
	EXPECT_EQ(
		_T("0b 10000000 00000000 00000000 00000001 (bit=31,0)"),
		Calculator::FormatBinaryResult(_T("0b10000000000000000000000000000001")));
	EXPECT_EQ(
		_T("0b 11111111 (bit=7,6,5,4,3,2,1,0)"),
		Calculator::FormatBinaryResult(_T("0b11111111")));
}

// 電卓機能として評価してよい式
TEST(CalculatorTest, AcceptsArithmeticExpression)
{
	EXPECT_TRUE(Calculator::IsValidExpression(_T("1+1")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("1 + 1")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("2*3")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("10/2")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("10%3")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("10//3")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("1+2*3")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("(1+2)*3")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("-5")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("3.14")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T(".5")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("1e3")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("0xFF")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("0b101")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("0o17")));
}

// 電卓機能として評価してよい式(mathの関数・定数)
TEST(CalculatorTest, AcceptsMathFunctionAndConstant)
{
	EXPECT_TRUE(Calculator::IsValidExpression(_T("sqrt(16)")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("math.sqrt(16)")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("math.pi")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("pi")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("atan2(1,2)")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("min(1,2)")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("max(1,2)")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("abs(-1)")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("round(1.5)")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("int(1.5)")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("float(1)")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("hex(255)")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("oct(255)")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("bin(255)")));
}

// 電卓機能として評価してよい式(べき乗)
TEST(CalculatorTest, AcceptsPowerExpression)
{
	EXPECT_TRUE(Calculator::IsValidExpression(_T("2**10")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("2 ** 10")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("2**(10)")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("2**0.5")));
	EXPECT_TRUE(Calculator::IsValidExpression(_T("(1+2)**3")));
}

// 任意のコードを実行できる式を拒否する
TEST(CalculatorTest, RejectsCodeExecutionExpression)
{
	// import
	EXPECT_FALSE(Calculator::IsValidExpression(_T("__import__('os')")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("import os")));

	// 組み込み関数
	EXPECT_FALSE(Calculator::IsValidExpression(_T("chr(65)")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("ord('a')")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("eval('1')")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("exec('import os')")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("compile('1','','eval')")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("open('test.txt')")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("print(1)")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("getattr(1,'x')")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("setattr(1,'x',1)")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("globals()")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("locals()")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("vars()")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("dir()")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("type(1)")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("object()")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("list()")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("dict()")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("set()")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("tuple()")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("str()")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("bytes()")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("input()")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("help()")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("breakpoint()")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("id(1)")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("hash(1)")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("sum((1,2))")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("sorted((1,2))")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("map(int,(1,2))")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("range(10)")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("iter((1,2))")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("next((1,2))")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("isinstance(1,int)")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("callable(1)")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("repr(1)")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("format(1)")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("ascii('a')")));

	// dunder属性を経由したオブジェクト模型的操作
	EXPECT_FALSE(Calculator::IsValidExpression(_T("hex.__self__")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("hex.__self__.__dict__")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("__builtins__")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("__class__")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("__base__")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("__subclasses__")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("__init__")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("__code__")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("__globals__")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("__dict__")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("(x for x in [1]).gi_frame.f_builtins")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("[].__class__.__base__.__subclasses__()")));

	// 引用符/セミコロン等の文字
	EXPECT_FALSE(Calculator::IsValidExpression(_T("'os'")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("\"os\"")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("1;import os")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("1==1")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("[1,2]")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("{'a':1}")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("lambda:1")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("1 if 1 else 2")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("f'{1}'")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("1&2")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("1|2")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("1^2")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("~1")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("1!=2")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("1<=2")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("x=1")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("1 2")));

	// 空文字列
	EXPECT_FALSE(Calculator::IsValidExpression(_T("")));
}

// 評価結果が巨大になり得る式を拒否する
TEST(CalculatorTest, RejectsOversizedExpression)
{
	// 指数の指定がないべき乗
	EXPECT_FALSE(Calculator::IsValidExpression(_T("10**10**10")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("2**100000000")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("2**(10**10)")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("2**1e5")));

	// 桁数の多い数値リテラル
	EXPECT_FALSE(Calculator::IsValidExpression(_T("1234567890123456789012345678901234567890")));
	EXPECT_FALSE(Calculator::IsValidExpression(_T("1e1000")));

	// 長い式
	CString longExpr;
	for (int i = 0; i < 300; ++i) {
		longExpr += _T("1+");
	}
	longExpr += _T("1");
	EXPECT_FALSE(Calculator::IsValidExpression(longExpr));
}
