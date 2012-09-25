Program tracker lze pouzit pri sledovani objektu. Tracker se spousti pres prikazovou radku
a dalsi jeho uziti uz je ciste pres jednoduche uzivatelske prostredi.
Tracker se spousti prikazem:

.\tracker -i vstupniVideo -o vystupniVideo

Vstupni video je video, ve kterem chceme objekt sledovat. Vystupni video je pouze
cesta a nazev, kde bude vysledne video ulozeno.
Ke spusteni programu je potreba mit nainstalovane OpenCV minimalne verze 2.2.
Format vstupniho videa zavisi od OpenCV a pouzitych kodeku, je doporuceno .avi.

Po spusteni je nejdrive potreba vybrat objekt pro sledovani. To se provede
zastavenim videa zmacknutim klavesy t a nasledneho vyberu objektu mysi. Opetovne spusteni
videa se provede po smacknuti mezerniku.
Pokud je potreba vybrat novy objekt, postup se opakuje a dale bude sledovan novy objekt.

Je potreba vzdy vybrat oblast po zmacknuti t, jinak program nebude fungovat validne. 
(jak je zmineno vyse, zmenit oblast neni problem)

Uzivatel dale muze menit hodnotu kappa, tedy hodnotu z rovnice statistickeho modelu. Tato
hodnota je mezi 0-1, na slidebarru je vsak znazornena hodnotami mezi 0 - 1000. 

Vhodne video, ze ktereho jsem vychazali lze nalest zde:
ftp://motinas.elec.qmul.ac.uk/pub/iLids/AVSS_PV_Easy_Divx.avi