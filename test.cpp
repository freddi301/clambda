// Copyright 2016 <Frederik Batuna>

#include "./lambda.cpp"

int main() {
  Lambda* Identity = l(i("x"), i("x"));
  std::cout << "Identity = " << Identity << std::endl;
  const Term* const test1 = a(Identity, Identity);
  std::cout << "I(I) = " << test1 << std::endl;
  std::cout << "I(I) => " << e(test1) << std::endl;
  const Term* const True = l(i("a"), l(i("b"), i("a")));
  const Term* const False = l(i("a"), l(i("b"), i("b")));
  std::cout << "True = " << True << std::endl;
  std::cout << "False = " << False << std::endl;
  std::cout << "(a=>b=>c=>a)(I)(T)(F) = " << e(a(a(a(l(i("a"), l(i("b"), l(i("c"), i("a")))), Identity), True), False)) << std::endl;
  std::cout << "(True(Identity))(False) => " << e(a(a(True, Identity), False)) << std::endl;
  std::cout << "True = " << True << " :: " << t(True) << std::endl;
  std::cout << "False = " << False << " :: " << t(False) << std::endl;
  std::cout << "(True Identity) = " << a(True, Identity) << " :: " << t(a(True, Identity)) << std::endl;
  std::cout << "(Identity False) = " << a(Identity, False) << " :: " << t(a(Identity, False)) << std::endl;
  const Term* const AAA = l(i("a"), a(i("a"), i("a")));
  std::cout << "AAA = " << AAA << /*" :: " << t(AAA) <<*/ std::endl; // todo infinite loop
  std::cout << "(AAA True) = " << e(a(AAA,True)) << std::endl;
  // std::cout << "(AAA AAA) = " << e(a(AAA,AAA)) << std::endl; // todo infinete type
  const Term* const Not = l(i("p"), a(a(i("p"), False), True));
  std::cout << "Not = " << Not << " :: " << t(Not) << std::endl;
  std::cout << "Not(True) = " << e(a(Not, True)) << " :: " << t(e(a(Not, True)).lambda) << std::endl; // todo implicitly evaluate
  const Term* const aI = l(i("a"), a(i("a"), l(i("x"),i("x"))));
  std::cout << aI << " :: " << t(aI) <<std::endl;
  const Term* const ABAB = l(i("a"), l(i("b"), a(i("a"), i("b"))));
  std::cout << ABAB << " :: " << t(ABAB) << std::endl;
  return 0;
}
