functor
import System
define
	local Test Next Filter in
		proc {Test N H C}
		   if N < C+1 then H = N else
			  if (N mod C) == 0 then H = 0 else
			 {Test N H C+1}
			  end
		   end
		end
		proc {Next L N C}
		  if C == N then L = nil
		   else H T in
			  L = H|T
			  thread {Test N H 2} end
			  {Next T N+1 C}
		   end
		end
		fun {Filter Xs F}
		   case Xs of nil then nil
		   [] X|Xr then if {F X} then X|{Filter Xr F} else {Filter Xr F} end
		   end
		 end
		local List List2 in
		   {Next List 20000000 20000100}
		   {Filter List fun {$ A} A>0 end List2}
		   {Browse List2}
		end
	end
end