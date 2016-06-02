define void @add1024(i1024* noalias %pz, i1024* noalias %px, i1024* noalias %py)
{
; version 3.8
  %x = load i1024, i1024* %px
  %y = load i1024, i1024* %py

; version 3.6
;  %x = load i1024* %px
;  %y = load i1024* %py

  %z = add i1024 %x, %x
  store i1024 %z, i1024* %pz
  ret void
}

