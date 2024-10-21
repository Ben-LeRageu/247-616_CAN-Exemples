#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <signal.h>

void vCanReceive(int fdSocketCAN, struct can_frame frame);
void vCantransmit(int fdSocketCAN, struct can_frame frame, pid_t pid);

int main(int argc, char **argv)
{
    int fdSocketCAN; 
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct can_frame frame;

    /*
	La première étape est de créer un socket. 
	Cette fonction accepte trois paramètres : 
		domaine/famille de protocoles (PF_CAN), 
		type de socket (raw ou datagram) et 
		protocole de socket. 
	la fonction retourne un descripteur de fichier.
	*/
	if ((fdSocketCAN = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("Socket");
		return -1;
	}
	
	/*
	Ensuite, récupérer l'index de l'interface pour le nom de l'interface (can0, can1, vcan0, etc.) 
	que nous souhaitons utiliser. Envoyer un appel de contrôle d'entrée/sortie et 
	passer une structure ifreq contenant le nom de l'interface 
	*/
	if(argc == 2)
		strcpy(ifr.ifr_name, argv[1]);
	else strcpy(ifr.ifr_name, "vcan0" );

	ioctl(fdSocketCAN, SIOCGIFINDEX, &ifr);
	/* 	Alternativement, zéro comme index d'interface, permet de récupérer les paquets de toutes les interfaces CAN.
	Avec l'index de l'interface, maintenant lier le socket à l'interface CAN
	*/

	/*
	
	*/
	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(fdSocketCAN, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Bind");
		return -1;
	}

    pid_t pid = fork();
    if(pid <0)
    {
        perror("Erreur lors de la création du processus fils");
    }

    if(pid == 0)
    {
        vCanReceive(fdSocketCAN, frame);
        close(fdSocketCAN);
        exit(EXIT_SUCCESS);
    }
    else
    {
        vCantransmit(fdSocketCAN, frame, pid);
        //kill(pid, SIGKILL);  // Terminer le processus fils
        //close(fdSocketCAN);
    }
    return 0 ;
}

void vCanReceive(int fdSocketCAN, struct can_frame frame)
{
    int nbytes, i;
  nbytes = read(fdSocketCAN, &frame, sizeof(struct can_frame));

 	if (nbytes < 0)
     {
		perror("Read");
		//return -1;
	}
    printf("Je suis le processus, voici ce que je reçois: \r");
	printf("0x%03X [%d] ",frame.can_id, frame.can_dlc);
	for (i = 0; i < frame.can_dlc; i++)
		printf("%02X ",frame.data[i]);
	printf("\r\n");

	if (close(fdSocketCAN) < 0) 
    {
		perror("Close");
		//return -1;
	}   
}

void vCantransmit(int fdSocketCAN, struct can_frame frame, pid_t pid)
{
  frame.can_id = 0x145;
  frame.can_dlc = 7;
  int option;
  while (1) 
  {
    printf("\n=== Menu ===\n");
    printf("1. Envoyer un message CAN\n");
    printf("2. Quitter\n");
    printf("Choix : ");
    scanf("%d", &option);
    if(option == 1)
    {
      // Préparation des données (exemple arbitraire)
      for (int i = 0; i < frame.can_dlc; i++) 
      {
        frame.data[i] = getchar();
      }
                
      // Envoi du message CAN
      int nbytes = write(fdSocketCAN, &frame, sizeof(struct can_frame));
      if (nbytes < 0)
      {
        perror("Erreur lors de l'envoi du message CAN");
      } 
      else 
      {
        printf("[Père] Message CAN envoyé : ID=0x%X, Data= ", frame.can_id);
        for (int i = 0; i < frame.can_dlc; i++) 
        {
          printf("%02X ", frame.data[i]);
        }
        printf("\n");
      }
      //printf("Je suis le processus père, voici ce que j'envoie \r");
      /*
	  Envoyer une trame CAN, initialiser une structure can_frame et la remplir avec des données. 
	  La structure can_frame de base est définie dans include/linux/can.h  
	  */
	  //frame.can_id = 0x145;  	// identifiant CAN, exemple: 247 = 0x0F7
	  //frame.can_dlc = 7;		// nombre d'octets de données
	  //sprintf(frame.data, "616-TGE");  // données 

	  //if (write(fdSocketCAN, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
      //{ 
		// perror("Write");
		 //return -1;
//	  }

	 // if (close(fdSocketCAN) < 0)
      //{
		//perror("Close");
		//return -1;
	  //}
      }
      else if (option == 2) 
      {
        printf("Arrêt du programme.\n");
        kill(pid, SIGKILL);  // Terminer le processus fils
        option = 0;
                break;
            } 
            else 
            {
                printf("Option invalide, veuillez réessayer.\n");
            }
    }
    close(fdSocketCAN);
    
}