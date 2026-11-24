# ECS Entity and Registry Lifetime

## Problem

The original `Entity` wrapper stored a raw `Registry*`. An entity wrapper could outlive the registry facade and then dereference a dangling pointer from `AddComponent`, `GetComponent`, `HasComponent`, `RemoveComponent`, or its boolean conversion.

## Ownership model

The ECS wrapper now stores its EnTT registry in a shared `RegistryState`.

- `Registry` owns one `std::shared_ptr<RegistryState>`.
- Each `Entity` created from a registry shares that state.
- Destroying the `Registry` facade releases only its ownership; the EnTT storage remains alive while entity wrappers still reference it.
- Destroying an entity through `Registry::Destroy` still invalidates its EnTT generation. Existing wrappers then evaluate to false.
- Component queries on invalid entities return false; component removal on invalid entities is a no-op.
- Component access and insertion on invalid entities throw `std::logic_error` instead of dereferencing invalid storage.

This prevents a registry-facade use-after-free without changing the public constructor shape (`Entity(EntityID, Registry*)`).

## Constraints

The shared state provides lifetime safety, not thread safety. Concurrent mutation and access to the same EnTT registry still require external synchronization. References returned by component accessors remain subject to EnTT storage invalidation rules.

The registry wrapper is deliberately non-copyable and non-movable. This avoids accidental facade copies that share mutable ECS state without an explicit ownership decision.

## Regression coverage

The unit tests cover:
- an entity remaining usable after its registry facade is destroyed;
- an entity becoming invalid after its underlying EnTT entity is destroyed;
- component access through the retained state.
