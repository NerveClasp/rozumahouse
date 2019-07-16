package go_server

import (
	"context"
) // THIS CODE IS A STARTING POINT ONLY. IT WILL NOT BE UPDATED WITH SCHEMA CHANGES.

type Resolver struct{}

func (r *Resolver) Mutation() MutationResolver {
	return &mutationResolver{r}
}
func (r *Resolver) Query() QueryResolver {
	return &queryResolver{r}
}

type mutationResolver struct{ *Resolver }

func (r *mutationResolver) ToggleLed(ctx context.Context, mac string, led int, ledOn bool) (*Device, error) {
	panic("not implemented")
}
func (r *mutationResolver) ChangeLedAnimation(ctx context.Context, mac string, led int, animation string) (*Device, error) {
	panic("not implemented")
}
func (r *mutationResolver) ChangeLedBrightness(ctx context.Context, mac string, led int, brightness int) (*Device, error) {
	panic("not implemented")
}
func (r *mutationResolver) ChangeLed(ctx context.Context, mac string, led int, brightness *int, animation *string, color []*ColorInput) (*Device, error) {
	panic("not implemented")
}
func (r *mutationResolver) Reboot(ctx context.Context, mac string) (*Device, error) {
	panic("not implemented")
}
func (r *mutationResolver) CheckForUpdates(ctx context.Context, mac string) (*Device, error) {
	panic("not implemented")
}
func (r *mutationResolver) SetActiveLeds(ctx context.Context, mac string, led int, activeLeds int) (*Device, error) {
	panic("not implemented")
}

type queryResolver struct{ *Resolver }

func (r *queryResolver) Device(ctx context.Context, ip *string, mac *string) (*Device, error) {
	panic("not implemented")
}
func (r *queryResolver) Devices(ctx context.Context, model *string) ([]*Device, error) {
	panic("not implemented")
}
