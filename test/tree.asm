extern Write

; fun {FoldR Xs P Z}
; 	 case Xs of nil then Z
;   [] X|Xr then {P X {FoldR Xr P Z}}
;   end
; end

def FoldR(3,0)
	match nil, s.0
	branch acc, c0
	match @|@, s.0
	push s.1
	push s.2
	call FoldR
	call s.1
c0:	ret 1	

; fun {Append Xs Ys}
;   case Xs of nil then Ys
;   [] X|Xr then X|{Append Xr Ys}
;   end
; end

def Append(2,0)
	match nil, s.0
	branch acc, c0
	match @|@, s.0
	push s.1
	call Append
	bind @,!(s.2|s.3)
c0:	ret 1

; fun {Concat Xs}
;	{FoldR Xs Append nil}
; end

def Concat(1,0)
	push Append
	push nil
	rcall FoldR

; fun {Preorder T}
;	case T of e then nil
;	[] n(V L R) then
;		{Concat [[V] {Preorder L} {Preorder R}]}
;	end
; end

def Preorder(1,0)
	match e, s.0
	branch acc, end
	match n(@ @ @), s.0
	call Preorder
	push s.2
	call Preorder
	bind @,![[s.1] s.4 s.3]
	rcall Concat
end: push nil
	ret 1
 
; fun {Inorder T}
;	case T of e then nil
;	[] n(V L R) then
;		{Concat [{Inorder L} [V] {Inorder R}]}
;	end
; end

def Write2(1,0)
	push s.0
	call Write
	ret

def Inorder(1,0)
	match e, s.0
	branch acc, c0
	match n(@ @ @), s.0
	call Inorder
	push s.2
	call Inorder
	bind @,![s.4 [s.1] s.3]
	rcall Concat
c0: push nil
	ret 1
	
; fun {Postorder T}
;	case T of e then nil
;	[] n(V L R) then
;		{Concat [{Inorder L} {Inorder R} [V]]}
;	end
; end

def Postorder(1,0)
	match e, s.0
	branch acc, end
	match n(@ @ @), s.0
	call Postorder
	push s.2
	call Postorder
	bind @,![s.4 s.3 [s.1]]
	rcall Concat
end: push nil
	ret 1


; fun {Levelorder T}
;	{Collect [T]}
; end

def Levelorder(1,0)
	bind @,![s.0]
	rcall Collect

; fun {Collect Queue}
;	case Queue of nil then nil
;		[] e|Xr then {Collect Xr}
;		[] n(V L R)|Xr then	V|{Collect {Append Xr [L R]}}
;	end
; end

def Collect(1,0)
	match e|@, s.0
	branch acc, c0
	match n(@ @ @)|@, s.0
	branch acc, c1
	ret 1
c0:	rcall Collect
c1: bind @,![s.2 s.3]
	call Append
	call Collect
	bind @,!(s.1|s.4)
	ret 1
	
; local Tree in
;	Tree = n(1 n(2 n(4 n(7 e e) e) n(5 e e)) n(3 n(6 n(8 e e) n(9 e e)) e))
;	{Show {Preorder Tree}}
;	{Show {Inorder Tree}}
;	{Show {Postorder Tree}}
;	{Show {Levelorder Tree}}
; end
	
def Main(0,0)	
	push n(269 n(745 n(776 n(618 n(975 n(446 n(8 n(895 n(412 n(851 n(62 n(97 n(797 n(168 n(617 n(670 n(945 n(993 n(457 n(304 n(699 n(795 n(992 n(984 n(134 n(642 n(569 n(320 n(665 n(628 n(678 n(368 n(516 n(750 n(440 n(31 n(433 n(392 n(556 n(271 n(926 n(9 n(553 n(940 n(113 n(871 n(704 n(676 n(93 n(178 n(888 n(332 n(709 n(235 n(669 n(234 n(133 n(307 n(981 n(829 n(875 n(218 n(290 n(655 n(293 n(929 n(422 n(192 n(798 n(774 n(72 n(379 n(282 n(852 n(889 n(167 n(987 n(922 n(310 n(644 n(722 n(9 n(75 n(377 n(798 n(496 n(754 n(675 n(68 n(279 n(601 n(86 n(49 n(710 n(519 n(848 n(588 n(364 n(226 n(684 n(638 n(769 n(189 n(976 n(252 n(446 n(601 n(174 n(858 n(167 n(931 n(793 n(303 n(790 n(607 n(697 n(8 n(602 n(385 n(869 n(572 n(194 n(137 n(624 n(684 n(464 n(488 n(684 n(910 n(472 n(221 n(425 n(269 n(518 n(23 n(164 n(301 n(430 n(418 n(737 n(911 n(899 n(457 n(201 n(536 n(289 n(151 n(511 n(299 n(431 n(502 n(935 n(511 n(550 n(23 n(833 n(235 n(388 n(200 n(790 n(462 n(984 n(139 n(394 n(234 n(620 n(573 n(552 n(363 n(673 n(713 n(149 n(504 n(367 n(250 n(48 n(675 n(366 n(150 n(150 n(906 n(227 n(106 n(971 n(789 n(31 n(386 n(607 n(587 n(618 n(662 n(943 n(275 n(313 n(999 n(634 n(525 n(280 n(11 n(802 n(101 n(612 n(951 n(938 n(766 n(24 n(87 n(250 n(79 n(167 n(512 n(362 n(465 n(802 n(851 n(69 n(321 n(574 n(991 n(498 n(657 n(956 n(535 n(243 n(262 n(158 n(545 n(872 n(598 n(585 n(162 n(733 n(749 n(732 n(281 n(689 n(734 n(834 n(341 n(822 n(148 n(435 n(818 n(703 n(844 n(970 n(852 n(63 n(596 n(152 n(456 n(873 n(916 n(213 n(163 n(255 n(601 n(651 n(248 n(147 n(613 n(45 n(741 n(455 n(161 n(292 n(906 n(103 n(643 n(546 n(239 n(597 n(599 n(31 n(705 n(726 n(204 n(285 n(334 n(392 n(264 n(399 n(148 n(430 n(96 n(580 n(896 n(506 n(227 n(583 n(881 n(534 n(354 n(397 n(723 n(489 n(118 n(14 n(174 n(135 n(317 n(504 n(431 n(926 n(101 n(519 n(957 n(776 n(374 n(632 n(883 n(872 n(475 n(252 n(567 n(891 n(92 n(54 n(248 n(785 n(888 n(708 n(147 n(922 n(487 n(70 n(862 n(61 n(90 n(181 n(933 n(111 n(524 n(787 n(714 n(339 n(414 n(365 n(68 n(334 n(944 n(467 n(838 n(471 n(184 n(711 n(486 n(417 n(980 n(450 n(897 n(524 n(455 n(882 n(650 n(693 n(459 n(984 n(203 n(449 n(984 n(255 n(314 n(673 n(977 n(870 n(677 n(23 n(964 n(206 n(514 n(214 n(900 n(607 n(477 n(213 n(503 n(269 n(734 n(228 n(582 n(700 n(523 n(364 n(620 n(551 n(97 n(690 n(265 n(512 n(157 n(4 n(839 n(362 n(910 n(454 n(336 n(367 n(233 n(615 n(140 n(751 n(631 n(970 n(448 n(113 n(564 n(109 n(15 n(220 n(481 n(710 n(220 n(903 n(819 n(480 n(866 n(362 n(189 n(219 n(182 n(167 n(950 n(562 n(668 n(434 n(608 n(457 n(610 n(814 n(205 n(762 n(440 n(106 n(666 n(551 n(543 n(65 n(184 n(603 n(410 n(242 n(310 n(687 n(934 n(271 n(196 n(608 n(50 n(821 n(40 n(452 n(24 n(253 n(414 n(583 n(624 n(590 n(595 n(977 n(851 n(967 n(255 n(750 n(455 n(289 n(487 n(620 n(521 n(34 n(28 n(691 n(774 n(679 n(320 n(818 n(114 n(976 n(150 n(398 n(194 n(425 n(832 n(44 n(707 n(426 n(466 n(889 n(272 n(493 n(955 n(434 n(976 n(367 n(420 n(929 n(410 n(174 n(589 n(864 e e) e) e) n(443 e e)) n(559 e e)) e) n(183 e e)) e) e) n(173 e e)) e) e) e) e) e) n(355 e e)) e) n(169 e e)) n(493 e e)) n(929 e e)) n(724 e e)) e) n(264 e e)) e) n(302 e e)) e) n(1000 e e)) e) n(843 e e)) e) e) e) n(23 e e)) e) n(727 e e)) e) e) e) e) n(87 e e)) n(823 e e)) e) n(883 e e)) e) n(567 e e)) n(33 e e)) e) n(735 e e)) e) e) e) n(632 e e)) n(940 e e)) n(771 e e)) n(711 e e)) n(55 e e)) n(514 e e)) e) e) e) e) n(969 e e)) n(621 e e)) e) n(378 e e)) n(512 e e)) e) n(123 e e)) n(263 e e)) n(420 e e)) n(842 e e)) n(803 e e)) e) n(158 e e)) e) e) e) n(316 e e)) n(637 e e)) n(733 e e)) n(851 e e)) n(823 e e)) n(505 e e)) e) e) n(427 e e)) n(540 e e)) e) n(165 e e)) n(709 e e)) e) e) e) n(971 e e)) e) e) e) n(801 e e)) n(356 e e)) e) n(617 e e)) e) n(40 e e)) e) e) e) e) n(885 e e)) e) n(249 e e)) e) e) n(839 e e)) e) n(27 e e)) e) e) n(378 e e)) n(975 e e)) e) n(330 e e)) e) e) e) e) n(930 e e)) n(467 e e)) n(577 e e)) n(88 e e)) n(305 e e)) n(858 e e)) e) n(239 e e)) e) n(930 e e)) n(492 e e)) e) n(733 e e)) e) e) e) n(597 e e)) e) n(886 e e)) e) e) e) e) n(577 e e)) e) e) n(869 e e)) n(832 e e)) n(621 e e)) e) e) n(71 e e)) e) e) e) n(214 e e)) n(481 e e)) e) e) e) e) n(304 e e)) n(324 e e)) e) n(541 e e)) n(27 e e)) e) e) n(467 e e)) e) n(84 e e)) e) e) n(634 e e)) e) n(666 e e)) e) n(898 e e)) n(248 e e)) n(18 e e)) n(338 e e)) e) e) e) n(658 e e)) e) e) n(244 e e)) e) n(167 e e)) e) n(248 e e)) n(603 e e)) e) e) n(770 e e)) e) n(297 e e)) n(15 e e)) e) e) n(369 e e)) n(64 e e)) n(42 e e)) e) n(284 e e)) e) n(489 e e)) e) n(248 e e)) e) e) n(756 e e)) n(466 e e)) e) e) e) n(460 e e)) n(850 e e)) e) e) e) n(214 e e)) e) n(137 e e)) n(368 e e)) n(467 e e)) n(854 e e)) e) e) e) e) n(112 e e)) e) n(361 e e)) n(241 e e)) n(197 e e)) e) n(958 e e)) n(5 e e)) n(727 e e)) e) e) e) n(504 e e)) e) n(707 e e)) n(82 e e)) e) e) n(733 e e)) n(781 e e)) e) n(927 e e)) n(151 e e)) e) n(835 e e)) n(858 e e)) e) e) n(377 e e)) e) e) e) n(968 e e)) n(587 e e)) n(26 e e)) n(514 e e)) e) e) n(609 e e)) n(575 e e)) e) e) e) n(864 e e)) e) e) e) n(773 e e)) n(198 e e)) e) e) e) e) n(622 e e)) e) e) n(667 e e)) e) n(917 e e)) e) e) e) n(721 e e)) n(247 e e)) e) n(877 e e)) n(815 e e)) e) e) e) n(566 e e)) n(117 e e)) n(345 e e)) n(306 e e)) n(645 e e)) e) e) n(936 e e)) e) n(633 e e)) n(435 e e)) n(756 e e)) n(317 e e)) e) e) e) n(289 e e)) n(183 e e)) n(432 e e)) n(758 e e)) e) e) e) n(963 e e)) e) n(646 e e)) n(436 e e)) n(639 e e)) n(806 e e)) e) e) n(590 e e)) n(765 e e)) n(89 e e)) n(281 e e)) e) n(665 e e)) e) e) e) n(99 e e)) e) e) e) n(248 e e)) e) e) e) e) e) e) e) e) n(940 e e)) e) n(456 e e)) e) n(963 e e)) n(136 e e)) e) n(90 e e)) n(88 e e)) n(662 e e)) n(805 e e)) e) n(129 e e)) n(191 e e)) n(914 e e)) n(804 e e)) e) n(583 e e)) n(69 e e)) e) e) n(986 e e)) e) e) n(955 e e)) n(690 e e)) e) e) e) e) n(23 e e)) e) e) e) e) e) n(39 e e)) e) n(917 e e)) n(47 e e)) e) e) n(552 e e)) n(909 e e)) e) n(402 e e)) e) e) n(476 e e)) e) n(41 e e)) e) e) e) e) n(383 e e)) n(952 e e)) n(255 e e)) n(647 e e)) e) e) n(939 e e)) e) e) n(501 e e)) n(224 e e)) e) n(240 e e)) n(627 e e)) e) n(274 e e)) e) n(325 e e)) e) e) n(962 e e)) e) e) e) e) n(941 e e)) n(57 e e)) e) e) e) e) n(147 e e)) n(254 e e)) e) e) n(321 e e)) e) n(997 e e)) e) e) e) e) n(944 e e)) e) n(128 e e)) e) e) n(12 e e)) e) n(462 e e)) n(5 e e)) e) e) n(524 e e)) n(682 e e)) n(607 e e)) n(610 e e)) e) e) e) e) e) n(318 e e)) n(57 e e)) n(742 e e)) n(627 e e)) e) n(778 e e)) e) e) e) n(948 e e)) e) e) e) n(594 e e)) n(539 e e)) e) e) n(618 e e)) n(62 e e)) n(103 e e)) e) e) n(509 e e))
	push s.0 call Preorder call Write
	push s.0 call Inorder call Write
	push s.0 call Postorder call Write
	push s.0 call Levelorder call Write
	ret