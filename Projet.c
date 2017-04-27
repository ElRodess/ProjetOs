#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

#define BUF 20
#define MINI 0.0 // pour negatif mettre -100000.0
#define MAXI 1000000.0
////////////////////////////////////////////////////////////////////////
/////////////////////////////Fonctions Directeur////////////////////////
////////////////////////////////////////////////////////////////////////
void init_chaine(char c[BUF])
{
	int i;
	for(i=0;i<BUF;i++)
	{
		c[i] = '\0'; // erreur normale tkt
	}
}

double max_directeur(double *T,int nombre)
{
	int i;
	double res = MINI;
	for (i=0;i<nombre;i++)
	{
		if (T[i] > res)
		{
			res=T[i];
		}
	}
	return res;
}

double min_directeur(double *T,int nombre)
{
	int i;
	double res = MAXI;
	for (i=0;i<nombre;i++)
	{
		if (T[i] < res)
		{
			res=T[i];
		}
	}
	return res;
}

double avg_directeur(double *T,int nombre)
{
	int i;
	double res = 0;
	for (i=0;i<nombre;i++)
	{
		
			res=res+T[i];
		
	}
	res=res/nombre;
	return res;
}

double sum_directeur(double *T,int nombre)
{
	int i;
	double res = 0;
	for (i=0;i<nombre;i++)
	{

			res=res+T[i];

	}
	return res;
}
///////////////////////////////////////////////////////////////////////
/////////////////////////////////Threads///////////////////////////////
///////////////////////////////////////////////////////////////////////
struct Donnee
{        
        int debut;
        int fin;
        double* tab;
        pthread_t p;
        pthread_mutex_t* mutex;
        double* valeur;
};
typedef struct Donnee Donnee;

Donnee init(int debut, int fin, double* tab, pthread_mutex_t* mutex, double* valeur)
{
	Donnee d;
	d.debut=debut;
	d.fin=fin;
	d.tab=tab;
	d.mutex=mutex;
	d.valeur=valeur;
	return d;
}

 int Compare(char* c1, char* c2){  
 	int i; 
    if(strlen(c1) != strlen(c2))
    { 
         return 0; 
 	} 
	for(i=0;i<strlen(c1);i++){ 
    if(c1[i] != c2[i])
    { 
             return 0; 
    } 
 	} 
     return 1; 
} 


void* max(void* arg)
{ 
        int j;
        double *max = malloc(sizeof(double));
        *max = MINI;
        Donnee donnee  = *(Donnee *)arg;
        for(j = donnee.debut; j < donnee.fin; j++)
        {
                if(donnee.tab[j] > *max)
                {
                        *max = donnee.tab[j];
                }
        }
        pthread_mutex_lock(donnee.mutex);
        if (*max > *donnee.valeur) 
        {        
                *donnee.valeur = *max;
        }
        pthread_mutex_unlock(donnee.mutex);
        return(NULL);
}

                
void* min(void* arg)

{ 
        int j;
        double *min = malloc(sizeof(double));
        *min = MAXI;
        Donnee donnee  = *(Donnee *)arg;
        for(j = donnee.debut; j < donnee.fin; j++)
        {
                if(donnee.tab[j] < *min)
                {
                        *min = donnee.tab[j];
                }
        }
        pthread_mutex_lock(donnee.mutex);
        if (*min < *donnee.valeur) 
        {        
                *donnee.valeur = *min;
        }
        pthread_mutex_unlock(donnee.mutex);
        return(NULL);
}
 
void* avg(void* arg)// résultat final en int 

{ 
        int j;
        int a;
        a=0;
        double *avg = malloc(sizeof(double));
        Donnee donnee  = *(Donnee *)arg;
        for(j = donnee.debut; j < donnee.fin; j++)
        {
              *avg=*avg+donnee.tab[j];
        }
			  *avg=*avg/(donnee.fin-donnee.debut);
        pthread_mutex_lock(donnee.mutex);
        if (a==0) 
        {        
                *donnee.valeur = *donnee.valeur+*avg;
                a++;
        }
        pthread_mutex_unlock(donnee.mutex);
        return(NULL);
} 

void* sum(void* arg)
{ 
        int j;
        int a;
        a=0;
        double *sum = malloc(sizeof(double));
        Donnee donnee  = *(Donnee *)arg;
        for(j = donnee.debut; j < donnee.fin; j++)
        {
              *sum=*sum+donnee.tab[j];
        }
        pthread_mutex_lock(donnee.mutex);
        if (a==0) 
        {        
                *donnee.valeur = *donnee.valeur+*sum;
                a++;
        }
        pthread_mutex_unlock(donnee.mutex);
        return(NULL);
} 

void* odd(void* arg)
{ 
        int j;
        int a = 0; 
        int odd = 0;
        Donnee donnee  = *(Donnee *)arg;
        for(j = donnee.debut; j < donnee.fin; j++)
        {
			if((int)donnee.tab[j]%2 == 1)
			{
				odd+=1;
            }
        }
        pthread_mutex_lock(donnee.mutex);
        if (a==0) 
        {        
                *donnee.valeur = *donnee.valeur+odd;
                a++;
        }
        pthread_mutex_unlock(donnee.mutex);
        return(NULL);
} 

// ctrl + e pour mettre les affichages

int main(int argc, char **argv)
{
	int k;
	int pid;
	//commandes : min max avg sum odd
	assert(argc >= 2);
	//Si y a pas de fichier ou la cmd pas alide on annule tout 
	assert(!strcmp(argv[1],"min") || !strcmp(argv[1],"max") || !strcmp(argv[1],"avg") || !strcmp(argv[1],"sum") || !strcmp(argv[1],"odd")); 
	double Resultats[argc-2];
	pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;//mutex pour eviter erreur à la lecture du pipe
	int fd[2]; // pipe
	int status;
	char buffer[BUF];
	for( k = 2; k < argc; k++) // on crée autant de process que de fichiers
	{
		pipe(fd);
		pid = fork();
		if ( pid == 0)
		{
			int fp; // fichier
			int i=0;
//////////////////////////////////////////////////////////////////////////////////////////////////
			fp = open( argv[k], O_RDONLY);
			char c[BUF];
////////////////////////////////////////////////////////////////////////
///////////////////////////Recuperation de la Taille Tableau////////////
////////////////////////////////////////////////////////////////////////
			char tmp = '0';
			while(tmp != '\n') // on recup la taille (premiere ligne)
			{
				c[i] = tmp;
				read(fp,&tmp,1);
				i++;
			}
			int Taille = atoi(c);
			init_chaine(c);
//////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////Remplissage-Tableau////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
			int j = 0;
			double Tab[Taille];
			tmp = '0';
			for(i=0;i<Taille;i++)
			{
				while(tmp != '\n')
				{
					c[j] = tmp;
					read(fp,&tmp,1);
					j++;
				}
				Tab[i] = atof(c);
				init_chaine(c);
				j = 0 ;
				tmp = '0';
			}	
/////////////////////////////////////////////////////////////////////
///////////////////////////Chefs d'équipes///////////////////////////
/////////////////////////////////////////////////////////////////////
			close(fp);
			int m = (Taille/100)+1;
			Donnee z[m];
			double res = 0.0;
			double* pointeur = malloc(sizeof(double));
			pthread_mutex_t mut= PTHREAD_MUTEX_INITIALIZER;
			if(Compare(argv[1],"min")) *pointeur = MAXI;
			else *pointeur = MINI;
			for(i=0; i< m; i++)
			{
				z[i]=init((i*(Taille/m)),(i+1)* (Taille/m),Tab,&mut,pointeur);
				if(Compare(argv[1],"max"))
				{
					pthread_create(&z[i].p,NULL,max,&z[i]);
				}
				if(Compare(argv[1],"min"))
				{
					pthread_create(&z[i].p,NULL,min,&z[i]);
				}
				if(Compare(argv[1],"avg"))
				{
					pthread_create(&z[i].p,NULL,avg,&z[i]);
				}
				if(Compare(argv[1],"sum"))
				{
					pthread_create(&z[i].p,NULL,sum,&z[i]);
				}
				if(Compare(argv[1],"odd"))
				{
					pthread_create(&z[i].p,NULL,odd,&z[i]);
				}	
			}
			for(i=0; i< m; i++)
			{
                pthread_join(z[i].p,NULL);
			} 
			if(Compare(argv[1],"avg"))
			{
				res=(*z[m-1].valeur/m);
			}
			else
			{
				res=*z[m-1].valeur;
			}
			//entrée de la variable dans le pipe
			init_chaine(c);
			sprintf(c,"%f",res);
			close(fd[0]);
			write(fd[1],c,BUF);
			close(fd[1]);
			exit(getpid()%10);
			free(pointeur);
		}
		else
		{
			init_chaine(buffer);
			close(fd[1]);
			pid = wait(&status); //Parce qu'il n'y a pas qu'un seul fils. 
						     	//Pour etre sur de retourner le pid du fils.
			pthread_mutex_lock(&mutex1);//on bloque // A ne pas faire apparement
			read(fd[0],buffer,BUF);
			Resultats[k-2] = atof(buffer);
			pthread_mutex_unlock(&mutex1); // <=
			close(fd[0]);
			if (!WIFEXITED(status))
				{
				printf("ERROR fichier : %d\n",k-1);
				return(0);
		    	}
		}
	}
	double final = 0.0;
	if(Compare(argv[1],"max"))final = max_directeur(Resultats,argc-2);
	if(Compare(argv[1],"min"))final = min_directeur(Resultats,argc-2);
	if(Compare(argv[1],"avg"))final = avg_directeur(Resultats,argc-2);
	if(Compare(argv[1],"sum")||Compare(argv[1],"odd"))final = sum_directeur(Resultats,argc-2);
	printf("%s : %f \n",argv[1],final);	
	return 0;
}
