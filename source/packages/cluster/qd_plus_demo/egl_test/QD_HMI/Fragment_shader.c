#ifdef GL_ES

#ifdef GL_FRAGMENT_PRECISION_HIGH
precision	highp		float; 
precision	highp		int;
#else
precision	mediump		float; 
precision	mediump		int;
#endif

#endif


const	int	c_pickup_offset_int		= 2;
const	int	c_pickup_cnt_int		= 5;


const	int c_zero_int				= 0;
const	int c_one_int				= 1;
const	float c_zero_float			= 0.0;
const	float c_one_float			= 1.0;
const	float c_min_float			= 0.000001;

/*[0--30)*/
const	int	draw_image_type			= 0;
const	int	draw_image_list_type	= 1;
const	int	draw_text_type			= 2;
const	int	draw_scroll_type		= 3;
const	int	draw_cube_image_type	= 4;
const	int	draw_spline_image_type	= 5;
const	int	draw_image_a8_type		= 6;/*2020 06 08*/

/*fill color [30--60)*/
const	int	draw_cube_color_type	= 30;
const	int	draw_fill_color_type	= 31;
const	int	draw_spline_color_type	= 32;
const	int	draw_cube_3d_color_type	= 33;
const	int	draw_cube_3d_texture_type= 34;
const	int	draw_etc1_alpha_type		= 35;

const	int	draw_3d_model_color_type		= 36;
const	int	draw_3d_model_texture_type		= 37;
const	int	draw_3d_model_pickup_type		= 38;
const	int	draw_3d_model_one_color_type	= 39;
const	int	draw_3d_model_line_type			= 40;
const	int	draw_3d_model_point_type		= 41;
const	int	draw_draw_border_type			= 42;	
const	int	draw_3d_model_shadow_type		= 43;


/*custom [60--160)*/
const	int	draw_center_scale_custom_type		= 60;
const	int	hmi_mask_enable					= 512;


const	int	hmi_gaussion					= 8;
const	int	hmi_a8_frmbuf					= 16;
const	int	hmi_shadow_soft					= 32;
const	int	hmi_cast_on						= 64;
const	int	hmi_multisample4				= 128;


const float PI								= 3.14159265359;
const float float_tolence					= 0.0000001;



/***************varying***********/
varying		vec2		v_texCoord;   
varying		vec2		v_etc1_alpha_texCoord;
varying		vec4		v_color;

varying			float			ba8frmbuf;
varying			float			bshadow_soft;
varying			float			bmultisample4;
varying			float			bshadow_on;
varying			float			bgauss_blur;

uniform		vec3		u_camera_pos; 			/*camera position*/	
uniform		int			u_light_type;			/*0:direct,1:spot light,2:point,-1:no light,-1:pickup*/
uniform		int			u_light_shadow_type;	/*0:No shadow,1:soft shadow,2:hard shadow*/

/********Draw type******/
uniform		float		u_alpha;
uniform		int			u_draw_type;
uniform		int			u_attribute;

uniform		int			screen_w;
uniform		int			screen_h;
uniform		vec4		u_one_color;

/*******stand texture*****/
uniform sampler2D	s_texture; 
uniform sampler2D	s_font_texture; 

/*******custom texture*****/
uniform sampler2D	s_texture_custom0;  /* used as MASK*/ 
uniform sampler2D	s_texture_custom1; 
/*blur*/
uniform		vec2		resolution;
uniform		float		blur_size;

uniform		int			u_big_img_w;
uniform		int			u_big_img_h;
uniform		int			u_small_img_x;
uniform		int			u_small_img_y;
uniform		int			u_small_img_w;
uniform		int			u_small_img_h;

uniform		int			u_qd;/*1:run at qd plus.0:run at API*/

#if 0
vec4 gauss_blur(void)
{
	vec2	texe_size;
	vec2	direction;
	vec2	new_texCoord;
	vec4	color	= vec4(c_zero_float,c_zero_float,c_zero_float,c_zero_float);
	float	fw;
	float	fh;	
	float	fsmall_w;
	float	fsmall_h;

	if((resolution.x > 0.0)&&(resolution.y > 0.0))
	{
		texe_size	= c_one_float / resolution;
		direction	= vec2(blur_size * texe_size.x,c_zero_float);
		//direction	= vec2(blur_size * texe_size.x,blur_size * texe_size.x);

		if(u_qd == c_zero_int)/*run at API*/
		{
			fw	= float(u_big_img_w);
			fh	= float(u_big_img_h);

			fsmall_w	= float(u_small_img_w);
			fsmall_h	= float(u_small_img_h);
			if((fsmall_w	 > c_zero_float)&&(fsmall_h > c_zero_float))
			{
				new_texCoord.x	= (v_texCoord.x * fw - float(u_small_img_x)) / 
									fsmall_w;
				new_texCoord.y	= c_one_float - ((c_one_float - v_texCoord.y) * fh - float(u_small_img_y)) / 
													fsmall_h;
				
			}
			else
			{
				new_texCoord.x	= 0.0;
				new_texCoord.y	= 0.0;
			}
		}
		else
		{
			new_texCoord.x	= v_texCoord.x;
			new_texCoord.y	= v_texCoord.y;
		}
		
		color += texture2D(s_texture_custom1, new_texCoord - 4.0 * direction) * 0.05;
	    color += texture2D(s_texture_custom1, new_texCoord - 3.0 * direction) * 0.09;
	   	color += texture2D(s_texture_custom1, new_texCoord - 2.0 * direction) * 0.12;
	   	color += texture2D(s_texture_custom1, new_texCoord - direction) * 0.15;
	    color += texture2D(s_texture_custom1, new_texCoord) * 0.16;
	    color += texture2D(s_texture_custom1, new_texCoord + direction) * 0.15;
	    color += texture2D(s_texture_custom1, new_texCoord + 2.0 * direction) * 0.12;
	    color += texture2D(s_texture_custom1, new_texCoord + 3.0 * direction) * 0.09;
	    color += texture2D(s_texture_custom1, new_texCoord + 4.0 * direction) * 0.05;
	}
	
			
	return color;
}
#else
vec4 gauss_blur(void)
{
	vec2	texe_size;
	vec2	direction;
	vec2	new_texCoord;
	vec4	color	= vec4(c_zero_float,c_zero_float,c_zero_float,c_zero_float);
	float	fw;
	float	fh;	
	float	fsmall_w;
	float	fsmall_h;
	float	weightSum = 0.0;
	float	x;
	float	y;
	float	xOffset;
	float	weight;
	float	yOffset;
	float	blur_size_sequal;

	if((resolution.x > 0.0)&&(resolution.y > 0.0))
	{
		blur_size_sequal	= blur_size * blur_size;
		texe_size	= c_one_float / resolution;
		direction	= vec2(blur_size * texe_size.x,blur_size * texe_size.y);
		
		if(u_qd == c_zero_int)/*run at API*/
		{
			fw	= float(u_big_img_w);
			fh	= float(u_big_img_h);

			fsmall_w	= float(u_small_img_w);
			fsmall_h	= float(u_small_img_h);
			if((fsmall_w	 > c_zero_float)&&(fsmall_h > c_zero_float))
			{
				new_texCoord.x	= (v_texCoord.x * fw - float(u_small_img_x)) / 
									fsmall_w;
				new_texCoord.y	= c_one_float - ((c_one_float - v_texCoord.y) * fh - float(u_small_img_y)) / 
													fsmall_h;
				
			}
			else
			{
				new_texCoord.x	= 0.0;
				new_texCoord.y	= 0.0;
			}
		}
		else
		{
			new_texCoord.x	= v_texCoord.x;
			new_texCoord.y	= v_texCoord.y;
		}
		
		for(x = -blur_size; x <= blur_size; x += c_one_float)
		{
			xOffset = x * direction.x;
			for (y = -blur_size; y <= blur_size; y += c_one_float)
			{
				yOffset = y * direction.y;
				weight = exp(-(xOffset * xOffset + yOffset * yOffset) / 
							(2.0 * blur_size_sequal)) / (2.0 * PI * blur_size_sequal);
				color += texture2D(s_texture_custom1, new_texCoord + vec2(xOffset, yOffset)) * weight;
				weightSum += weight;
			}
		}
		if(weightSum > float_tolence)
		{
			color = color / weightSum;
		}
		else
		{
			color = texture2D(s_texture_custom1, new_texCoord);
		}
	}
	
			
	return color;
}

#endif









void main()                                         
{    
	vec4		texture_alpha;
	vec4		texture_color;
	vec4		font_texture_color;
	vec4		mask_alpha;
	vec2		mask_uv;	
	vec4		pick_up_color;
	int			i;
	int			j;
	float		row_begin;
	float		col_begin;
	bool		finished;
	vec2		pickup_uv;	
	vec4		blur_color;
	vec2		new_texCoord;
	int			iattribute;
	
	iattribute	= u_attribute;
	/*MASK*/	
	if(u_attribute >= hmi_mask_enable)
	{
		mask_uv.x		= gl_FragCoord.x / (float(screen_w));
		mask_uv.y		= gl_FragCoord.y / (float(screen_h));	
		mask_alpha		= texture2D(s_texture_custom0, mask_uv);	
		iattribute		-= hmi_mask_enable;		
	}*/

	
	

			
	/*normal texture*/
	if((u_draw_type != draw_text_type)&&	
		(u_draw_type < draw_cube_color_type))
	{
		texture_color	= texture2D(s_texture, v_texCoord);
	}
	/*font text*/
	else if(u_draw_type == draw_text_type)
	{
		font_texture_color	= texture2D(s_font_texture, v_texCoord);
	}
	else if(u_draw_type == draw_etc1_alpha_type)/*ect1*/
	{
		texture_color	= texture2D(s_texture, v_texCoord);
		texture_alpha	= texture2D(s_texture, v_etc1_alpha_texCoord);
	}	
	else if(u_draw_type == draw_3d_model_texture_type)
	{
		if(u_qd == c_zero_int)/*run at API*/ 
		{
			if(float(u_big_img_w) > c_zero_float)
			{
				new_texCoord.x	= (float(u_small_img_x) + 
									float(u_small_img_w) * v_texCoord.x) / float(u_big_img_w);
			}
			if(new_texCoord.x > c_one_float)
			{
				new_texCoord.x = c_one_float;
			}
			if(float(u_big_img_h) > c_zero_float)
			{		
				new_texCoord.y	= c_one_float - ((float(u_small_img_y) + 
									float(u_small_img_h) * v_texCoord.y) / float(u_big_img_h));		
			}
			if(new_texCoord.y > c_one_float)
			{
				new_texCoord.y = c_one_float;
			}
			texture_color	= texture2D(s_texture, new_texCoord);
			
		}
		else	/*run at qd tool*/
		{
			texture_color	= texture2D(s_texture, v_texCoord);			
		}
	}	
	else if(u_draw_type == draw_cube_3d_texture_type)/*3d cube model*/
	{		
		texture_color	= texture2D(s_texture, v_texCoord);		
	}	
	else
	{
	}

	if(u_draw_type < draw_cube_color_type)/*fill image*/  
	{		
		if(u_draw_type != draw_text_type)/*not text*/
		{		
				if(u_draw_type != draw_image_a8_type)/*not a8 image*/
				{				
					texture_color.a	= texture_color.a * u_alpha;

					if(texture_color.a > c_zero_float)
					{
						gl_FragColor	= texture_color;
					}
					else
					{
						discard;
					}
				}
				else	/*A8 image*/   
				{		
					if(ba8frmbuf > c_zero_float)
					{
						texture_alpha	= u_one_color;				
						texture_alpha.a	= texture_color.a * texture_alpha.a;
						gl_FragColor	= texture_alpha;
					}
					else
					{
						gl_FragColor.a	= texture_color.a;
					}								
				}  
			}
			else	/*gauss blur*/
			{				
												
											
			texture_color			= u_one_color;	 		
			texture_color.a			= font_texture_color.a * texture_color.a;
			gl_FragColor			= texture_color;  																	
		}		
	}	
	else if(u_draw_type == draw_etc1_alpha_type)/*ect1 alpha*/
	{			
		texture_color.a	= texture_alpha.r * u_alpha;
		//gl_FragColor	= texture_color; 
		if(texture_color.a > c_zero_float)
		{
			gl_FragColor	= texture_color;
		}
		else
		{
			discard;
		}
	}  
	else if(u_draw_type == draw_spline_color_type)/*spline color*/
	{
		gl_FragColor = u_one_color;		
		gl_FragColor.a = gl_FragColor.a;						
	}
	else if(u_draw_type == draw_cube_3d_color_type)/*3d color cube model*/
	{	
	}
	else if(u_draw_type == draw_cube_3d_texture_type)/*3d cube model*/
	{		
	}  
	else if(u_draw_type == draw_3d_model_color_type)
	{
		gl_FragColor	= v_color;
	}
	else if(u_draw_type == draw_3d_model_shadow_type)
	{				
		if(bshadow_soft > c_zero_float)
		{
			gl_FragColor	= v_color;	
			gl_FragColor.a	=(v_color.r + v_color.g + v_color.b) / 3.0;
		}
		else
		{
			gl_FragColor	= v_color;	
			gl_FragColor.a	= (v_color.r + v_color.g + v_color.b) / 3.0;
		}	
	}
	else if(u_draw_type == draw_3d_model_texture_type)
	{	
		if(u_light_type >= 0)/*light enable*/
		{
			if(bshadow_on > c_zero_float)
			{			
				texture_color.a	= texture_color.a;
				gl_FragColor	= texture_color * v_color;				
				/*gl_FragColor	= texture_color + v_color;*/
			
			}
			else
			{								
				texture_color.a	= texture_color.a * u_alpha ;					
				if(texture_color.a > c_zero_float)
				{
					gl_FragColor	= texture_color;
				}
				else
				{
					discard;
				}				
			}
		}
		else if(u_light_type == -1)/*no light*/
		{		
			texture_color.a	= texture_color.a;
			gl_FragColor	= texture_color;			
		}
		else	/*pickup*/
		{
			gl_FragColor	= u_one_color;
		}
	}	
	else if(u_draw_type == draw_3d_model_one_color_type)
	{
		if(u_light_type >= 0)/*light enable*/
		{
			if(bshadow_on > c_zero_float)
			{
				texture_color.a	= v_color.a;
				gl_FragColor	= v_color;
				gl_FragColor.a	= texture_color.a;
			}
			else
			{
				texture_color.a	= u_one_color.a;
				gl_FragColor	= u_one_color;	
				gl_FragColor.a	= texture_color.a;
			}
		}
		else if(u_light_type == -1)/*no light*/
		{
			texture_color.a	= u_one_color.a;
			gl_FragColor	= u_one_color;	
			gl_FragColor.a	= texture_color.a;
		}
		else	/*pickup*/
		{
			gl_FragColor	= u_one_color;	
		}
	}
	else if(u_draw_type == draw_3d_model_pickup_type)
	{				
		gl_FragColor	= u_one_color;				
	}
	else if(u_draw_type == draw_3d_model_line_type)
	{
		gl_FragColor	= u_one_color;							
	}
	else if(u_draw_type == draw_3d_model_point_type)
	{
		gl_FragColor	= u_one_color;	
	}
	else if(u_draw_type == draw_draw_border_type)
	{
		pickup_uv.x		= gl_FragCoord.x / (float(screen_w));
		pickup_uv.y		= gl_FragCoord.y / (float(screen_h));	
		pick_up_color	= texture2D(s_texture, pickup_uv);

		gl_FragColor	= vec4(c_zero_float,c_zero_float,c_zero_float,c_zero_float);
		if((pick_up_color.x != u_one_color.x)||
			(pick_up_color.y != u_one_color.y)||
			(pick_up_color.z != u_one_color.z)||
			(pick_up_color.a != u_one_color.a))
		{
			row_begin	= gl_FragCoord.y - float(c_pickup_offset_int);
			col_begin	= gl_FragCoord.x - float(c_pickup_offset_int);
			finished	= false;
			for(i = c_zero_int;(i < c_pickup_cnt_int)&&(finished == false);i++)
			{			
				for(j = c_zero_int;j < c_pickup_cnt_int;j++)
				{
					pickup_uv.x		= (col_begin + float(j))  / (float(screen_w));	
					pickup_uv.y		= (row_begin + float(i)) / (float(screen_h));	
				
					if((pickup_uv.x >= c_zero_float)&&(pickup_uv.x <= c_one_float))
					{
						if((pickup_uv.y >= c_zero_float)&&(pickup_uv.y <= c_one_float))
						{
							pick_up_color	= texture2D(s_texture, pickup_uv);
							if((pick_up_color.x == u_one_color.x)&&
								(pick_up_color.y == u_one_color.y))
							{
								if((pick_up_color.z == u_one_color.z)&&
									(pick_up_color.a == u_one_color.a))
								{
									gl_FragColor	= vec4(c_one_float,c_zero_float,c_one_float,c_one_float);
									finished		= true;
									break;
								}						
							}					
						}				
					}				
				}			
			}
			if(finished == false)
			{
				discard;
			}
		}
		else
		{
			discard;
		}
	}
	else if(u_draw_type < draw_center_scale_custom_type)/*fill color*/
	{
		gl_FragColor	= v_color;		
		gl_FragColor.a	= gl_FragColor.a;			
	}
	else	/*custom*/
	{		
		gl_FragColor = vec4(c_one_float,c_one_float,c_one_float,mask_alpha.a);			
	}	
}                                                   














