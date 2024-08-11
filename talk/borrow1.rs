fn func<'a, 'b>(x:&'a mut i32, y:&'b mut i32) -> &'a mut i32 {
  return x;  
}

fn main() {
  let mut x = 1;
  let mut y = 2;
  let r = func(&mut x, &mut y);

  // The loan on x is live.
  // The loan on y is dead.
  // We can't mutate x.
  x += 1;

  // A use that extends the ^x loan.
  let z = *r;
}