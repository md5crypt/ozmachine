extern Write

; proc {Test N H C}
;   if N < C+1 then H = N else
;      if (N mod C) == 0 then H = 0 else
;	 {Test N H C+1}
;      end
;   end
;end

def Test(3,0)
	rpn-push 1
	rpn-push s.2
	rpn-add
	push acc
	match s.3, s.0
	branch acc, l0
	rpn-push 0
	rpn-push s.0
	rpn-push s.2
	rpn-mod
	rpn-eq
	branch acc, l1
	mov s.2, s.3
	pop 1
	rcall Test
l0: bind s.1,s.0
	ret
l1: bind s.1,0
	ret

; proc {Next L N C}
;  if C == N then L = nil
;   else H T in
;      L = H|T
;      thread {Test N H 2} end
;      {Next T N+1 C}
;   end
; end

def Next(3,0)
	match s.2,s.1
	branch acc, l0
	create 2
	bind s.0,!(s.3|s.4)
	push s.1
	push s.3
	push 2
	tcall Test
	rpn-push s.1
	rpn-push 1
	rpn-add
	push acc
	push s.2
	rcall Next
l0:	bind s.0,nil
	ret

; fun {Filter Xs F}
;   case Xs of nil then nil
;   [] X|Xr then if {F X} then X|{Filter Xr F} else {Filter Xr F} end
;   end
; end

def Filter(2,0)
	match s.0, nil
	branch acc, l0
	match @|@, s.0
	push s.2
	call s.1
	branch s.4, l1
	pop 1
	push s.1
	rcall Filter
l1:	mov s.0, s.2
	mov s.2, s.1
	mov s.1, s.3
	pop 2
	call Filter
	bind @,!(s.0|s.1)
	ret 1
l0: pop 1
	ret 1

; local List List2 in
;   {Next List 2000000 2100000}
;   {Filter List fun {$ A} A>0 end List2}
;   {Browse List2}
; end

def Anon(1,0)
	rpn-push s.0
	rpn-push 0
	rpn-ge
	push acc
	ret 1

def Main(0,0)
	create 1
	push s.0
	push 20000
	push 21000
	call Next
	push s.0
	push Anon
	call Filter
	call Write
	ret

