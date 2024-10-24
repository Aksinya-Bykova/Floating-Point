## 0.72 / 1.25
## Самое полезное - это список литературы и тесты. Есть умная идея положить в union обычный float и свой float. В целом написать код проще чем прочитать и понять

## Инструментарий

> С++17, Clang

## Вариант

> fixed normal, floating normal

# Полезные ссылки
1. Sterbenz Floating Point Computation, Sterbenz, Pat H., 1974,
     https://archive.org/details/SterbenzFloatingPointComputation/mode/2up
2. What Every Computer Scientist Should Know About Floating-Point Arithmetic,
     David Goldberg, 1991,
     https://cr.yp.to/2005-590/goldberg.pdf
3. Numerical Computation Guide, Sun Microsystems, Inc., 2003
     https://www5.in.tum.de/~huckle/numericalcomputationguide.pdf
4. IEEE Standard for Binary Floating-Point
     Arithmetic,The Institute of Electrical and Electronics Engineers, 1985
     https://www.ime.unicamp.br/~biloti/download/ieee_754-1985.pdf
5. "The Art of Assembly Language Programming"
     (CS 422/522: Design and Implementation of Operating Systems
     Fall 2022, Yale University, chapter 14)
     https://flint.cs.yale.edu/cs422/doc/art-of-asm/pdf/CH14.PDF
6. Karen Miller, 2006
     https://pages.cs.wisc.edu/~markhill/cs354/Fall2008/notes/flpt.apprec.html
7. https://en.wikipedia.org/wiki/Rounding
8. https://github.com/francof2a/fxpmath/tree/master

# Описание:

> ## Fixed point
> Округление выполняется следующим образом. Оно применяется везде, где есть потеря младших разрядов:
> как на стадии получения результата операции (при наличии), так и при переводе ответа в десятичную запись.
> "Округление производится до модулярной арифметики" означает, что во время операции сначала производится округление до
> необходимого бита, а потом делается взятие по модулю.
>
> **Округление к нулю:** потеря младших битов
>
> **Округление к ближайшему чётному:** математическое округление кроме ситуации точной половины
> (например, 1.50000_10 -> 2.00000_10), в таком случае производится округление к ближайшему чётному.
>
> **Округление к плюс бесконечности:** всегда округление вверх
>
> **Округление к минус бесконечности:** всегда округление вниз
>
> https://en.wikipedia.org/wiki/Rounding – пользовалась определениями отсюда
>

> Пример кода, генерирующего тесты для случая вывода фиксированной точки с округлением к нулю. Для других типов
> аналогично, только в ```res[]``` добавляются другие значения. ```round()``` – к ближайшему чётному,
> ```math.ceil()``` – к плюс бесконечности, ```math.floor()``` – к минус бесконечности.
>
> Операции в фиксированной точке выполняются следующим образом. Сложение и вычитание как сложение и вычитание
> чисел из A+B бит, а для получения десятичного результата нужно перевести число из дополнительного кода и считать
> последние B бит как дробную часть.
>
> Умножение и деление выполняются схожим образом. Пусть x, y – два числа, с которыми производится операция. Тогда
> справедливо, что (x * y) * 2^B = (x * 2^B * y * 2^B) / 2^B, (x / y) * 2^B = (x * 2^B * 2^B) / (y * 2^B). Округление
> получившегося результата определяется остатками от деления на 2^B или (y * 2^B) соответственно – смотрим,
> результат больше, равен или меньше точной половины делителя.
>
> Вычисления выполняются по модулю, а в конце учитывается знак.
>
> Округление соотвествующего типа для перевода в десятичную запись делается на основе
> сравнения B бит с 2^(B - 1) – смотрим, результат больше, равен или меньше точной половины. На основе этого выбираем,
> куда округлять тысячные.
>
> Для фиксированной точки создала класс FixedPoint, где реализованы методы операций, get_hexadecimal() – десятичный
> вывод,
> get_ab_bits() – взятие последних A+B битов (по условию может прийти больше и старшие необходимо проигнорировать). Для
> всего этого есть вспомогательные приватные методы, которые делают ровно вышеописанный алгоритм.
>

```c++
int a_size_;
int b_size_;

uint32_t actual_number_; // A + B bits

uint32_t sign_;

int32_t integer_part_value_; // A bits

uint32_t frac_part_value_; // B bits
```

> Поля кодируют размеры A, B, число без точки, A бит до точки, B бит после точки. Когда в программе нужно взять ровно
> A или B бит, я беру их логическими операциями, так что возникающие при промежуточных вычислениях лишние старшие биты
> мне не мешают. Для этого я использую следующий способ.
>

```c++
/* x - value
 * n - number of bits
 * */
x & ((1 << n) - 1); 
```

> ## Float point
> The IEEE single format consists of three fields: a 23-bit fraction, f; an 8-bit biased
> exponent, e; and a 1-bit sign, s. These fields are stored contiguously in one 32-bit
> word, as shown in FIGURE 2-1. Bits 0:22 contain the 23-bit fraction, f, with bit 0 being
> the least significant bit of the fraction and bit 22 being the most significant; bits 23:30
> contain the 8-bit biased exponent, e, with bit 23 being the least significant bit of the
> biased exponent and bit 30 being the most significant; and the highest-order bit 31
> contains the sign bit, s. [3, p. 25]
>
> Half precision определяется аналогично, только на мантиссу 10 бит, экспоненту 5 бит и на знак 1 бит.

**Half precision**

| 0   | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|
| +/- | e | e | e | e | e | m | m | m | m | m  | m  | m  | m  | m  | m  |

**Single precision**

| 0   | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|
| +/- | e | e | e | e | e | e | e | e | m | m  | m  | m  | m  | m  | m  |

| 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 |
|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
| m  | m  | m  | m  | m  | m  | m  | m  | m  | m  | m  | m  | m  | m  | m  | m  |

> Представление в виде кода

```c++
struct FloatNumber32 {
union {
float a_float_;
uint32_t int_bits_;
struct {
uint32_t mantissa_: 23;
uint32_t exponent_: 8;
uint32_t sign_: 1;
};
// ...
};
```

> В int_bits_ можно напрямую написать то, что передали в командной строке и поля с мантиссой, экспонентной и знаком
> автоматически заполнятся. a_float_ использую только для отладки, потому что это удобно.
>
> Случаи с бесконечностями, NaN, +-0 и денормализованными числами проверяю только на стадии вывода. Далее описано, что
> это за числа с точки зрения плавающей точки.

> TABLE shows the correspondence between the values of the three constituent
> fields s, e and f, on the one hand, and the value represented by the single- format
> bit pattern on the other; u means don’t care, that is, the value of the indicated field is
> irrelevant to the determination of the value of the particular bit patterns in single
> format. [3, p.26]
>

| Single-Format Bit Pattern                                | Value                                   |
|----------------------------------------------------------|-----------------------------------------|
| 0 < e < 255                                              | (–1)s × 2e–127 × 1.f (normal numbers)   |
| e = 0; f ≠ 0 (at least one bit in f is nonzero)          | (–1)s × 2–126 × 0.f (subnormal numbers) |
| e = 0; f = 0 (all bits in f are zero)                    | (–1)s × 0.0 (signed zero)               |
| s = 0; e = 255; f = 0 (all bits in f are zero)           | +INF (positive infinity)                |
| s = 1; e = 255; f = 0 (all bits in f are zero)           | –INF (negative infinity)                |
| s = u; e = 255; f ≠ 0 (at least one bit in f is nonzero) | NaN (Not-a-Number)                      |

> Notice that when e < 255, the value assigned to the single format bit pattern is
> formed by inserting the binary radix point immediately to the left of the fraction’s
> most significant bit, and inserting an implicit bit immediately to the left of the binary
> point, thus representing in binary positional notation a mixed number (whole
> number plus fraction, wherein 0 ≤ Ò3fraction < 1).
> The mixed number thus formed is called the single-format significand. The implicit bit
> is so named because its value is not explicitly given in the single- format bit pattern,
> but is implied by the value of the biased exponent field.
>
> For the single format, the difference between a normal number and a subnormal
> number is that the leading bit of the significand (the bit to left of the binary point) of
> a normal number is 1, whereas the leading bit of the significand of a subnormal
> number is 0. Single-format subnormal numbers were called single-format
> denormalized numbers in IEEE Standard 754.
> The 23-bit fraction combined with the implicit leading significand bit provides 24
> bits of precision in single-format normal numbers. [3, p.26]
>
> ### Арифметические операции
> Арифметические операции поняла по примерам [1, ch. 1], [2, p. 8].
>
> Сложение и вычитание означает сдвиг меньшего по модулю числа и сложение мантисс,
> экспонента при этом будет равна экспоненте большего по модулю (более точно см. код).
>
> Умножение и деление означает сложение (или вычитание) экспонент и умножение (деление) мантисс (более точно см. код).
>
> ### Невалидные операции
> The invalid operation exception is signaled if an operand is invalid for the operation on to be performed. The result,
> when the exception occurs without a trap, shall be a quiet NaN provided the destination has a floating-point
> format. The invalid operations are
> 1. Any operation on a signaling NaN
> 2. Addition or subtraction—magnitude subtraction of infinites such as, (+∞) + (−∞)
> 3. Multiplication—0 × ∞
> 4. Division—0/0 or ∞/∞
> 5. Remainder— x REM y, where y is zero or x is infinite
> 6. Square root if the operand is less than zero
> 7. Conversion of a binary floating-point number to an integer or decimal format when overflow, infinity, or NaN
     precludes a faithful representation in that format and this cannot otherwise be signaled
> 8. Comparison by way of predicates involving < or >, without ?, when the operands are unordered
>
> [4, p. 11]
> В нашей задаче примечательны правила 1-6.
> Из правила 1: если на входе подаётся сигнальный NaN, я вывожу nan и делаю выход из программы.
>
> ### Округление
> По умолчанию к ближайшему чётному [4, p. 5], но по задаче будем округлять в соответствии с тем,
> что было передано в командной строке. Я округляю тогда и только тогда, когда есть потеря младших битов.
>
> ### Переполнение вверх
> The correctly rounded
> result would be larger in
> magnitude than the
> largest finite number
> representable in the
> destination format (i.e.,
> the exponent range is
> exceeded). [3, p.75]
>
> 7.3 Overflow
> The overflow exception shall be signaled whenever the destination format’s largest finite number is exceeded in
> magnitude by what would have been the rounded floating-point result (Section 4) were the exponent range unbounded.
> The result, when no trap occurs, shall be determined by the rounding mode and the sign of the intermediate result as
> follows:
> 1. Round to nearest carries all overflows to ∞ with the sign of the intermediate result
> 2. Round toward 0 carries all overflows to the format's largest finite number with the sign ofthe intermediate
     result
> 3. Round toward −∞ carries positive overflows to the formats largest finite number, and carries negative
     overflows to −∞
> 4. Round toward +∞ carries negative overflows to the format’s most negative finite number, and carries positive
     overflows to +∞
     > [4, p.11]
>
> То есть переполнение вверх зависит от того, какой тип округления передали в командной строке.
>
> > ### Денормализованные числа
> У денормализованных чисел представление (-1)^s * 2^(-126) * 0.f [3, p.26]
> Поэтому при нормализации нужно вычитать именно 126.
>
> There are two important cases where a floating point number cannot be normalized.
> The value 0.0 is a special case. Obviously it cannot be normalized because the floating
> point representation for zero has no one bits in the mantissa. This, however, is not a problem
> since we can exactly represent the value zero with only a single bit.[5, p. 777]
>

> ### Переполнение вниз при умножении
> **f 0 0x00216143 * 0xBF1FBBA8**
>
> Переведём эти числа в двоичный вид и точкой отделим границы знак-экспонента, экспонента-мантисса.
> 0.00000000.01000010110000101000011 * 1.01111110.00111111011101110101000
>
> Теперь рассмотрим только мантиссы с математической точки зрения (точка как в математике).
> 0.01000010110000101000011 * 1.00111111011101110101000 = 00.0101001101001111100000100100001100010011111000
>
> Отсюда получаем экспоненциальную запись
> 0.0101001101001111100000100100001100010011111000_2 * 2^(-126 + (126 - 127)) =
>
> 0.0101001101001111100000100100001100010011111000_2 * 2^(-1) * 2^(-126)
>
> Степень -127 не записать, поэтому получим её дополнительным сдвигом мантиссы вправо.
> Нам сейчас нужно получить число с плавающей точкой в денормализованном виде. То есть
> 0.00101001101001111100000100100001100010011111000_2 * 2^(-126)
>
> **f 0 0x7F000001 * 0xC0000002**
> Аналогично разделим точками знак, мантиссу и экспоненту
> 0.00111100.01111001110010100001000 * 0.00111100.01111001110010100001000
>
> Перемножили мантиссы
> 1.01111001110010100001000 * 1.01111001110010100001000 = 10.0010110110000100110000101001110101000001000000
>
> Мантиссу необходимо сдвинуть так, чтобы целая часть была 1 (побитовый сдвиг вправо на 1),
> и за счёт этого экспоненту необходимо увеличить на 1.
>
> 60 + 60 + 1 - 127 = -6. То есть экспонента должна стать -133. В ответ пойдёт 0x1.16c200p-133
>
> **f 0 0x40000000 * 0x40886666**
>
> 0.10000000.00000000000000000000000_2 * 0.10000001.00010000110011001100110_2
>
> exp: 128 + 129 - 127 = 130
>
> Перемножим мантиссы. Экспоненту менять не придётся.
> 1.00000000000000000000000 * 1.00010000110011001100110 = 1.00010000110011001100110(00000000000000000000000)
>
> 00010000110011001100110 - мантисса в ответе без учёта единицы в целой части,
> вывод мантиссы 0x1.10cccc, так как нужно 24 бита в ответе
>
> **f 0 0x7F000001 * 0x80000002**
> Разделим точками знак, мантиссу и экспоненту
> 0.11111110.00000000000000000000001 * 1.00000000.00000000000000000000010_2
>
> Поскольку экспонента правого числа = 0, это денормализованное число, то есть целая часть мантиссы = 0.
> 1.00000000000000000000001 * 0.00000000000000000000010_2 = 0.00000000000000000000010(00000000000000000000010)
>
> 1.00000000000000000000001, -126 + (254 - 127) - 22 = -21 - экспонента
>
> Соответственно вывод с учётом 24 бит: -0x1.000002p-21


