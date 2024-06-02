# tuple_utility

## tuple_utilityはC++のメタプログラミング用ヘッダオンリーライブラリです．

std::tupleを加工する様々なユーティリティを提供します．

## 使用方法
### push_back
```
using tpl=tuple_utility::push_back<std::tuple<int>,float>::type
// expects: tpl=std::tuple<int,float>;
```

### transform
```
using tpl=tuple_utility::transform<std::tuple<int,float,char>,std::vector>::type;
// expects: tpl=std::tuple<std::vector<int>,std::vector<float>,std::vector<char>>;
```
### subtuple
```
using tpl = tuple_utility::subtuple<std::tuple<int,float,char,double>,0,2>::type;
// expects: tpl=std::tuple<int,float>;
```
### その他
他にも様々なユーティリティがあります．
