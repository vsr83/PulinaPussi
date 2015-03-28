int Pthread_mutex_lock(pthread_mutex_t *);
int Pthread_mutex_unlock(pthread_mutex_t *);
int Pthread_join(pthread_t, void **);
int Pthread_create(pthread_t *, const pthread_attr_t *, void *(*)(void *), 
		   void *);

int Inputf_getch();
int Inputf_clear();
int Curses_init ();
int Curses_clearall();

void finish(int sig);

