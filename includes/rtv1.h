#ifndef RTV1_H
# define RTV1_H

# include <stdlib.h>
# include <math.h>

typedef struct			s_vec
{
	float				x;
	float				y;
	float				z;
}						t_vec;

t_vec				set_vec(float x, float y, float z);

t_vec				vec_sub(t_vec v1, t_vec v2);

t_vec				vec_add(t_vec v1, t_vec v2);
t_vec				vec_add_f(t_vec v1, float value);

t_vec				vec_mult_f(t_vec v, float f);

t_vec				vec_mult(t_vec v1, t_vec v2);

float				dot_product(t_vec v1, t_vec v2);

float				vec_length2(t_vec vec);

t_vec				vec_normalize(t_vec vec);

float				max(float a, float b);
float				min(float a, float b);


#endif