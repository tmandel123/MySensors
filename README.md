# MySensors
MySensors Nodes 


Diese MySensors Quellen sind optimiert, um mit dem Controller FHEM zusammenzuarbeiten.
Es gibt Einstellungen, mit denen im laufenden Betrieb Debugging beim Node eingeschaltet werden kann.

Vorgesehen sind 2 Umgebungen: Produktion und Test

Produktion = Kanal 96
Test = Kanal 76

Updates der Nodes erfolgen mit FOTA über Kanal 76 (der Reboot wird im Kanal 96 initiiert und der Bootloader startet im Standard mit Kanal 76)



| First Header | Second Header |
| -------------| ------------- |
| Content Cell  | Content Cell  |
| Content Cell  | Content Cell  |
