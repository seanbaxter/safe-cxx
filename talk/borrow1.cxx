#feature on safety

auto func/(a, b)(int^/a x, int^/b y) noexcept safe -> int^/a {
  return x;
}

int main() safe {
  int x = 1;
  int y = 2;
  int^ r = func(^x, ^y);

  // The loan on x is live.
  // The loan on y is dead.
  // We can't mutate x.
  ++x;

  // A use that extends the ^x loan.
  int z = *r;
}
