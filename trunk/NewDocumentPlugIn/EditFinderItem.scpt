FasdUAS 1.101.10   ��   ��    k             l      ��  ��    � �
-- Edit Finder Item

This script select theItem from the front window of the
Finder, and display the edit textfield for this item.

Requires theItem to have been defined, and UI
elements to be enabled.

(c) Kemenaran 2009
     � 	 	� 
 - -   E d i t   F i n d e r   I t e m 
 
 T h i s   s c r i p t   s e l e c t   t h e I t e m   f r o m   t h e   f r o n t   w i n d o w   o f   t h e 
 F i n d e r ,   a n d   d i s p l a y   t h e   e d i t   t e x t f i e l d   f o r   t h i s   i t e m . 
 
 R e q u i r e s   t h e I t e m   t o   h a v e   b e e n   d e f i n e d ,   a n d   U I 
 e l e m e n t s   t o   b e   e n a b l e d . 
 
 ( c )   K e m e n a r a n   2 0 0 9 
   
  
 l     ��������  ��  ��        l     ����  O         I   �� ��
�� .miscslctnull���    obj   n        4    �� 
�� 
cobj  o   	 
���� 0 theitem theItem  4   �� 
�� 
cwin  m    ���� ��    m       �                                                                                  MACS   alis    r  Macintosh HD               Ğ@UH+     �
Finder.app                                                       s��0�        ����  	                CoreServices    Ğ@U      �0�       �   T   S  3Macintosh HD:System:Library:CoreServices:Finder.app    
 F i n d e r . a p p    M a c i n t o s h   H D  &System/Library/CoreServices/Finder.app  / ��  ��  ��        l     ��������  ��  ��     ��  l   : ����  O    :    Z    9  ����  1    ��
�� 
uien  k    5      !   I   !�� "��
�� .sysodelanull��� ��� nmbr " m     # # ?���������   !  $ % $ O  " / & ' & r   ) . ( ) ( m   ) *��
�� boovtrue ) 1   * -��
�� 
pisf ' 4   " &�� *
�� 
prcs * m   $ % + + � , ,  F i n d e r %  -�� - I  0 5�� .��
�� .prcskprsnull���    utxt . o   0 1��
�� 
ret ��  ��  ��  ��    m     / /�                                                                                  sevs   alis    �  Macintosh HD               Ğ@UH+     �System Events.app                                                ���C        ����  	                CoreServices    Ğ@U      ��C       �   T   S  :Macintosh HD:System:Library:CoreServices:System Events.app  $  S y s t e m   E v e n t s . a p p    M a c i n t o s h   H D  -System/Library/CoreServices/System Events.app   / ��  ��  ��  ��       �� 0 1��   0 ��
�� .aevtoappnull  �   � **** 1 �� 2���� 3 4��
�� .aevtoappnull  �   � **** 2 k     : 5 5   6 6  ����  ��  ��   3   4  �������� /�� #���� +������
�� 
cwin
�� 
cobj�� 0 theitem theItem
�� .miscslctnull���    obj 
�� 
uien
�� .sysodelanull��� ��� nmbr
�� 
prcs
�� 
pisf
�� 
ret 
�� .prcskprsnull���    utxt�� ;� *�k/��/j UO� &*�,E �j O*��/ e*�,FUO�j Y hU ascr  ��ޭ