#pragma once 

#include <cx_json_parser.h>

namespace tipi::build {
  using namespace JSON::literals;
  static constexpr auto openapi_json = R"x( 
{
	"openapi": "3.0.2",
	"info": {
		"title": "Swagger Petstore - OpenAPI 3.0",
		"description": "This is a sample Pet Store Server based on the OpenAPI 3.0 specification.",
		"termsOfService": "http://swagger.io/terms/",
		"contact": {
			"email": "apiteam@swagger.io"
		},
		"license": {
			"name": "Apache 2.0",
			"url": "http://www.apache.org/licenses/LICENSE-2.0.html"
		},
		"version": "1.0.6"
	},
	"externalDocs": {
		"description": "Find out more about Swagger",
		"url": "http://swagger.io"
	},
	"servers": [{
		"url": "/api/v3"
	}],
	"tags": [{
			"name": "pet",
			"description": "Everything about your Pets",
			"externalDocs": {
				"description": "Find out more",
				"url": "http://swagger.io"
			}
		},
		{
			"name": "store",
			"description": "Operations about user"
		},
		{
			"name": "user",
			"description": "Access to Petstore orders",
			"externalDocs": {
				"description": "Find out more about our store",
				"url": "http://swagger.io"
			}
		}
	],
	"paths": {
		"/pet": {
			"post": {
				"tags": [
					"pet"
				],
				"summary": "Add a new pet to the store",
				"description": "Add a new pet to the store",
				"operationId": "addPet",
				"requestBody": {
					"description": "Create a new pet in the store",
					"content": {
						"application/json": {
							"schema": {
								"$ref": "#/components/schemas/Pet"
							}
						},
						"application/xml": {
							"schema": {
								"$ref": "#/components/schemas/Pet"
							}
						},
						"application/x-www-form-urlencoded": {
							"schema": {
								"$ref": "#/components/schemas/Pet"
							}
						}
					},
					"required": true
				},
				"responses": {
					"200": {
						"description": "Successful operation",
						"content": {
							"application/xml": {
								"schema": {
									"$ref": "#/components/schemas/Pet"
								}
							},
							"application/json": {
								"schema": {
									"$ref": "#/components/schemas/Pet"
								}
							}
						}
					},
					"405": {
						"description": "Invalid input"
					}
				},
				"security": [{
					"petstore_auth": [
						"write:pets",
						"read:pets"
					]
				}]
			}
		}
	},
	"components": {
		"schemas": {

			"Pet": {
				"required": [
					"name",
					"photoUrls"
				],
				"type": "object",
				"properties": {
          "id": {
            "type": "integer",
            "format": "int64",
            "example": 10
          },
					"name": {
						"type": "string",
						"example": "doggie"
					},
					"photoUrls": {
						"type": "array",
						"xml": {
							"wrapped": true
						},
						"items": {
							"type": "string",
							"xml": {
								"name": "photoUrl"
							}
						}
					}
				},
				"xml": {
					"name": "pet"
				}
			}
		},
		"securitySchemes": {
			"petstore_auth": {
				"type": "oauth2",
				"flows": {
					"implicit": {
						"authorizationUrl": "https://petstore3.swagger.io/oauth/authorize",
						"scopes": {
							"write:pets": "modify pets in your account",
							"read:pets": "read your pets"
						}
					}
				}
			},
			"api_key": {
				"type": "apiKey",
				"name": "api_key",
				"in": "header"
			}
		}
	}
} 
  )x"_cx_json;

}