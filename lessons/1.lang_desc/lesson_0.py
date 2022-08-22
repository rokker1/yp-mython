a = "hello"
b = 'world'
c = 'long string with a double quote " inside'
d = "another long string with a single quote ' inside"
e = "string with a double quote \" inside"
f = 'string with a single quote \' inside'
g = ""
h = ''
print(a, b, c , d, e, f)

t = True
false = False

# это комментарий
x = 5 #это тоже комментарий
# в следующей строке # - обычный символ
hashtag = "#природа"

class Rect:
  def __init__(self, w, h):
    self.w = w
    self.h = h

  def area(self):
    return self.w * self.h

r = Rect(10, 5)
print(r.area())


x = 4        # переменная x связывается с целочисленным значением 4
# следующей командой переменная x связывается со значением 'hello'
x = 'hello'
y = True
x = y 
s = 'hello, ' + 'world'

class Fire:
  def __init__(self, obj):
    self.obj = obj

  def __str__(self):
    return "Burnt " + str(self.obj)

class Tree:
  def __str__(self):
    return "tree"

class Matches: # Спички
  # операция сложения спичек с другими объектами превращает их в огонь
  def __add__(self, smth):
    return Fire(smth)

result = Matches() + Tree()
print(result)             # Выведет Burnt tree
print(Matches() + result) # Выведет Burnt Burnt tree

class Person:
  def __init__(self, name, age):
    self.name = name
    self.age = age
  def __eq__(self, rhs):
    return self.name == rhs.name and self.age == rhs.age
  def __lt__(self, rhs):
    if self.name < rhs.name:
        return True
    return self.name == rhs.name and self.age < rhs.age
  def __le__(self, rhs):
    return ((self < rhs) or (self == rhs))

print (Person("Ivan", 10) <= Person("Sergey", 10)) # True
print (Person("Ivan", 10) <= Person("Sergey", 9))  # False

class Rect:
  def __init__(self, w, h):
    self.w = w
    self.h = h

  def __str__(self):
    return "Rect(" + str(self.w) + 'x' + str(self.h) + ')'

print(str(Rect(4, 3)))
str(Rect(4, 3))
x = 4
w = 'world'
print(x, x + 6, 'Hello, ' + w)