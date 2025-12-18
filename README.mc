Pour compiler le projet : 

./projet_ip chemin_instance i j 

où : 
i = 0 pour lancer CG avec pricing MIP
i = 1 pour lancer CG avec pricing DP 
i = 2 pour lancer CG avec pricing DP + stabilisation (il faudra saisir un paramètre alpha manuellement via un cin)

j = 1 activer l'écriture
j = 0 désactiver l'écriture



Pour le lancement de la fomulation compacte (décommenter le main) : 

./projet_ip chemin_instance i 

où: 
i = 1 si on active la relaxation linéaire
i = 0 si on la désactive


le fichier jupyter "reader" dans builds a servi a faire les stats 
le fichier jupyter "tests" dans builds a servi à automatiser les éxécutions
