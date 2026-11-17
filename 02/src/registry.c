#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "registry.h"

static SubscriberNode* find_subscriber(Registry* registry, pid_t pid) {
    SubscriberNode* current = registry->subscribers;

    while (current != NULL) {
        if (current->pid == pid) {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

int registry_register_publisher(Registry* registry, pid_t pid) {
    PublisherNode* current = registry->publishers;

    while (current != NULL) {
        if (current->pid == pid) {
            return 0;
        }

        current = current->next;
    }

    PublisherNode* publisher = malloc(sizeof(PublisherNode));

    if (publisher == NULL) {
        return -1;
    }

    publisher->pid = pid;
    publisher->next = registry->publishers;
    registry->publishers = publisher;

    return 0;
}

int subscriber_has_topic(const SubscriberNode* subscriber, const char* topic) {
    TopicNode* current = subscriber->topics;

    while (current != NULL) {
        if (strcmp(current->name, topic) == 0) {
            return 1;
        }

        current = current->next;
    }

    return 0;
}

int registry_add_subscription(Registry* registry, pid_t pid, const char* topic) {
    SubscriberNode* subscriber = find_subscriber(registry, pid);

    if (subscriber != NULL && subscriber_has_topic(subscriber, topic)) {
        return 0;
    }

    TopicNode* new_topic = malloc(sizeof(TopicNode));

    if (new_topic == NULL) {
        return -1;
    }

    strcpy(new_topic->name, topic);

    if (subscriber == NULL) {
        subscriber = malloc(sizeof(SubscriberNode));

        if (subscriber == NULL) {
            free(new_topic);
            return -1;
        }

        subscriber->pid = pid;
        subscriber->topics = NULL;
        subscriber->next = registry->subscribers;
        registry->subscribers = subscriber;
    }

    new_topic->next = subscriber->topics;
    subscriber->topics = new_topic;

    return 0;
}

void registry_remove_subscription(Registry* registry, pid_t pid, const char* topic) {
    SubscriberNode* subscriber = registry->subscribers;
    SubscriberNode* previous_subscriber = NULL;

    while (subscriber != NULL && subscriber->pid != pid) {
        previous_subscriber = subscriber;
        subscriber = subscriber->next;
    }

    if (subscriber == NULL) {
        return;
    }

    TopicNode* current_topic = subscriber->topics;
    TopicNode* previous_topic = NULL;

    while (current_topic != NULL && strcmp(current_topic->name, topic) != 0) {
        previous_topic = current_topic;
        current_topic = current_topic->next;
    }

    if (current_topic == NULL) {
        return;
    }

    bool is_selected_first_topic = (previous_topic == NULL);

    if (is_selected_first_topic) {
        subscriber->topics = current_topic->next;
    } else {
        previous_topic->next = current_topic->next;
    }

    free(current_topic);

    if (subscriber->topics != NULL) {
        return;
    }

    bool is_selected_first_subscriber = (previous_subscriber == NULL);

    if (is_selected_first_subscriber) {
        registry->subscribers = subscriber->next;
    } else {
        previous_subscriber->next = subscriber->next;
    }

    free(subscriber);
}

void registry_clear(Registry* registry) {
    SubscriberNode* subscriber = registry->subscribers;

    while (subscriber != NULL) {
        TopicNode* topic = subscriber->topics;

        while (topic != NULL) {
            TopicNode* next_topic = topic->next;
            free(topic);
            topic = next_topic;
        }

        SubscriberNode* next_subscriber = subscriber->next;
        free(subscriber);
        subscriber = next_subscriber;
    }

    PublisherNode* publisher = registry->publishers;

    while (publisher != NULL) {
        PublisherNode* next_publisher = publisher->next;
        free(publisher);
        publisher = next_publisher;
    }

    registry->subscribers = NULL;
    registry->publishers = NULL;
}