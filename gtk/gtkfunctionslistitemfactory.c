/*
 * Copyright © 2019 Benjamin Otte
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Benjamin Otte <otte@gnome.org>
 */

#include "config.h"

#include "gtkfunctionslistitemfactory.h"

#include "gtklistitemfactoryprivate.h"
#include "gtklistitemprivate.h"

struct _GtkFunctionsListItemFactory
{
  GtkListItemFactory parent_instance;

  GtkListItemSetupFunc setup_func;
  GtkListItemBindFunc bind_func;
  gpointer user_data;
  GDestroyNotify user_destroy;
};

struct _GtkFunctionsListItemFactoryClass
{
  GtkListItemFactoryClass parent_class;
};

G_DEFINE_TYPE (GtkFunctionsListItemFactory, gtk_functions_list_item_factory, GTK_TYPE_LIST_ITEM_FACTORY)

static void
gtk_functions_list_item_factory_setup (GtkListItemFactory *factory,
                                       GtkListItemWidget  *widget,
                                       GtkListItem        *list_item)
{
  GtkFunctionsListItemFactory *self = GTK_FUNCTIONS_LIST_ITEM_FACTORY (factory);

  if (self->setup_func)
    self->setup_func (list_item, self->user_data);

  GTK_LIST_ITEM_FACTORY_CLASS (gtk_functions_list_item_factory_parent_class)->setup (factory, widget, list_item);

  if (gtk_list_item_get_item (list_item) != NULL && self->bind_func)  
    self->bind_func (list_item, self->user_data);
}

static void                  
gtk_functions_list_item_factory_update (GtkListItemFactory *factory,
                                        GtkListItemWidget  *widget,
                                        GtkListItem        *list_item,
                                        guint               position,
                                        gpointer            item,
                                        gboolean            selected)
{
  GtkFunctionsListItemFactory *self = GTK_FUNCTIONS_LIST_ITEM_FACTORY (factory);

  GTK_LIST_ITEM_FACTORY_CLASS (gtk_functions_list_item_factory_parent_class)->update (factory, widget, list_item, position, item, selected);

  if (item != NULL && self->bind_func)  
    self->bind_func (list_item, self->user_data);
}

static void
gtk_functions_list_item_factory_finalize (GObject *object)
{
  GtkFunctionsListItemFactory *self = GTK_FUNCTIONS_LIST_ITEM_FACTORY (object);

  if (self->user_destroy)
    self->user_destroy (self->user_data);

  G_OBJECT_CLASS (gtk_functions_list_item_factory_parent_class)->finalize (object);
}

static void
gtk_functions_list_item_factory_class_init (GtkFunctionsListItemFactoryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkListItemFactoryClass *factory_class = GTK_LIST_ITEM_FACTORY_CLASS (klass);

  object_class->finalize = gtk_functions_list_item_factory_finalize;

  factory_class->setup = gtk_functions_list_item_factory_setup;
  factory_class->update = gtk_functions_list_item_factory_update;
}

static void
gtk_functions_list_item_factory_init (GtkFunctionsListItemFactory *self)
{
}

/**
 * gtk_functions_list_item_factory_new: (skip)
 * @setup_func: (scope call) (destroy user_destroy): the setup function for the list item factory
 * @bind_func: (scope call) (destroy user_destroy): the bind function for the list item factory
 * @user_data: user data for the @setup_func and @bind_func functions
 * @user_destroy: a function called to destroy @user_data
 *
 * Creates a new #GtkListItemFactory with the given functions.
 *
 * Returns: (transfer full): the newly created list item factory object
 */
GtkListItemFactory *
gtk_functions_list_item_factory_new (GtkListItemSetupFunc setup_func,
                                     GtkListItemBindFunc  bind_func,
                                     gpointer             user_data,
                                     GDestroyNotify       user_destroy)
{
  GtkFunctionsListItemFactory *self;

  g_return_val_if_fail (setup_func || bind_func, NULL);
  g_return_val_if_fail (user_data != NULL || user_destroy == NULL, NULL);

  self = g_object_new (GTK_TYPE_FUNCTIONS_LIST_ITEM_FACTORY, NULL);

  self->setup_func = setup_func;
  self->bind_func = bind_func;
  self->user_data = user_data;
  self->user_destroy = user_destroy;

  return GTK_LIST_ITEM_FACTORY (self);
}

