# mython
class GCD:
  def calc(a, b):
    if a == 0 or b == 0:
      return a + b
    else:
      if a < b:
        return self.calc(b, a)
      else:
        # В Mython нет операции нахождения остатка от деления,
        # поэтому эмулируем её через имеющиеся операции
        # a - a / b * b == a % b
        return self.calc(b, a - a / b * b)

  def is_coprime(a, b):
    return self.calc(a, b) == 1

coprime = False

gcd = None
gcd = GCD()

x = 4
y = 13
coprime = gcd.is_coprime(x, y)
if coprime:
  print x, 'and', y, 'are coprime'
else:
  print x, 'and', y, 'are not coprime'