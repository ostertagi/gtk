#ifndef __GTK_ARRAY_IMPL_PRIVATE_H__
#define __GTK_ARRAY_IMPL_PRIVATE_H__


/* This is a dumbed-down GPtrArray, which takes some stack
 * space to use. When using this, the general case should always
 * be that the number of elements is lower than reserved_size.
 * The GPtrArray should only be used in extreme cases.
 */

typedef struct
{
  guint reserved_size;
  guint len;
  void **stack_space;
  GPtrArray *ptr_array;

} GtkArray;


static inline void
gtk_array_init (GtkArray   *self,
                void     **stack_space,
                guint      reserved_size)
{
  self->reserved_size = reserved_size;
  self->len = 0;
  self->stack_space = stack_space;
  self->ptr_array = NULL;
}

static inline void *
gtk_array_index (const GtkArray *self,
                 guint           index)
{
  g_assert (index < self->len);

  if (G_LIKELY (!self->ptr_array))
    return self->stack_space[index];

  return g_ptr_array_index (self->ptr_array, index);
}

static inline void
gtk_array_add (GtkArray *self,
               void     *element)
{
  if (G_LIKELY (self->len < self->reserved_size))
    {
      self->stack_space[self->len] = element;
      self->len++;
      return;
    }

  /* Need to fall back to the GPtrArray */
  if (G_UNLIKELY (!self->ptr_array))
    {
      self->ptr_array = g_ptr_array_new_full (self->len + 1, NULL);
      memcpy (self->ptr_array->pdata, self->stack_space, sizeof (void *) * self->len);
      self->ptr_array->len = self->len;
    }

  g_ptr_array_add (self->ptr_array, element);
  self->len++; /* We still count self->len */
}

static inline void
gtk_array_insert (GtkArray *self,
                  guint     index,
                  void     *element)
{
  if (index >= self->len)
    {
      gtk_array_add (self, element);
      return;
    }

  if (G_LIKELY (self->len < self->reserved_size))
    {
      memmove (self->stack_space + index + 1, self->stack_space + index,
               sizeof (void *) * (self->len - index));
      self->stack_space[index] = element;
      self->len++;
      return;
    }

  if (G_UNLIKELY (!self->ptr_array))
    {
      self->ptr_array = g_ptr_array_new_full (self->len + 1, NULL);
      memcpy (self->ptr_array->pdata, self->stack_space, sizeof (void *) * self->len);
      self->ptr_array->len = self->len;
    }

  g_assert (self->ptr_array);
  g_ptr_array_insert (self->ptr_array, index, element);
  self->len++;
}

static inline void
gtk_array_free (GtkArray       *self,
                GDestroyNotify  element_free_func)
{
  guint i;

  if (G_LIKELY (!self->ptr_array))
    {
      if (element_free_func)
        {
          for (i = 0; i < self->len; i++)
            element_free_func (self->stack_space[i]);
        }

      return;
    }

  g_assert (self->ptr_array);

  if (element_free_func)
    {
      for (i = 0; i < self->ptr_array->len; i++)
        element_free_func (g_ptr_array_index (self->ptr_array, i));
    }

  g_ptr_array_free (self->ptr_array, TRUE);
}

static inline void **
gtk_array_get_data (GtkArray *self)
{
  if (G_LIKELY (!self->ptr_array))
    return self->stack_space;

  return self->ptr_array->pdata;
}


#endif
